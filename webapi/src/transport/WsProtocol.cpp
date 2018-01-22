/*
 * WsProtocol.cpp
 *
 *  Created on: 1/7/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <cstdint>
#include <list>

#include <core/ulog.h>
#include <core/ThreadHelper.hpp>

#include <webapi/websockets/server_ws.hpp>
#include <webapi/websockets/client_ws.hpp>
#include <webapi/transport/WsProtocol.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      namespace {

        template<class ConnectionSide>
        void sendMessageInternal(const std::shared_ptr<typename ConnectionSide::Connection>& connection,
                                 const WsMessage& wsMessage)
        {
          boost::property_tree::ptree tree = serialization::JsonSerializer<WsMessage>::Serialize(wsMessage);

          auto send_stream = std::make_shared<typename ConnectionSide::SendStream>();

          boost::property_tree::write_json(*send_stream, tree);

          connection->send(send_stream, [](const SimpleWeb::error_code &ec)
          {
            if(ec)
            {
              mdebug_error("Couldn't send message. Error: %s(%d).", ec.message().c_str(), ec);
            }
          });
        }

      }

      template <class ConnectionSide>
      WsProtocol<ConnectionSide>::WsProtocol(size_t resendNumber, int resendTimeout) :
              resendNumber_(resendNumber), resendTimeout_(resendTimeout), inCounter_(0), outCounter_(0), stopped(false)
      {

      }


      template <class ConnectionSide>
      size_t WsProtocol<ConnectionSide>::getInCounter()
      {
        return inCounter_;
      }

      template <class ConnectionSide>
      size_t WsProtocol<ConnectionSide>::getNextOutCounter()
      {
        return (outCounter_++)%SIZE_MAX;
      }

      template <class ConnectionSide>
      void WsProtocol<ConnectionSide>::sendMessage(const core::BaseMessage::Ptr& message, const std::shared_ptr<typename ConnectionSide::Connection>& connection)
      {
        WsData wsData(getNextOutCounter(), message);

        mdebug_notice("Going to send 'DATA' message: '%d'.", wsData.getId());
        sendMessageInternal<ConnectionSide>(connection, wsData);

        addToResendStore(connection, std::make_shared<MessageData>(connection, wsData));
      }

      template <class ConnectionSide>
      core::BaseMessage::Ptr WsProtocol<ConnectionSide>::parseMessage(const std::shared_ptr<typename ConnectionSide::Message> message,
                                                                      const std::shared_ptr<typename ConnectionSide::Connection>& connection)
      {
        core::BaseMessage::Ptr result;

        std::stringstream stringStream;
        stringStream << message->string();

        boost::property_tree::ptree tree;
        boost::property_tree::read_json(stringStream, tree);

        WsMessage::Ptr wsMessage = serialization::JsonDeserializer<WsMessage>::Parse(tree);

        if(wsMessage->getType() == WsMessageType::Ack)
        {
          mdebug_notice("Received 'ACK' message: '%s'.", stringStream.str().c_str());
          removeFromResendStore(wsMessage->getId());
        }
        else if(wsMessage->getType() == WsMessageType::Data)
        {
          auto wsData = std::static_pointer_cast<WsData>(wsMessage);
          result = wsData->getData();
          mdebug_info("Received 'DATA' message: '%s'.", stringStream.str().c_str());

          WsAck wsAck(wsMessage->getId());
          mdebug_notice("Going to send 'ACK' for: '%d'.", wsMessage->getId());
          sendMessageInternal<ConnectionSide>(connection, wsAck);
        }

        return result;
      }

      template<class ConnectionSide>
      void WsProtocol<ConnectionSide>::addToResendStore(
              const std::shared_ptr<typename ConnectionSide::Connection>& connection,
              const typename WsProtocol::MessageData::Ptr& messageData)
      {
        std::lock_guard<std::mutex> lck(resendStoreMutex);
        resendStore.insert({messageData->message.getId(), messageData});
        resendStoreCondVar.notify_one();
      }

      template<class ConnectionSide>
      void WsProtocol<ConnectionSide>::removeFromResendStore(WsMessageIdentifier messageId)
      {
        std::lock_guard<std::mutex> lck(resendStoreMutex);
        removeFromResendStoreUnsafe(messageId);
      }

      template<class ConnectionSide>
      void WsProtocol<ConnectionSide>::removeFromResendStoreUnsafe(WsMessageIdentifier messageId)
      {
        size_t erasedMessages = resendStore.erase(messageId);
        mdebug_notice("Going to remove message with id '%d' from the resend store.", messageId, core::ThreadHelper::threadIdToInt());

        if(erasedMessages != 1)
        {
          mdebug_warn("No such message in resend store with id: '%d'.", messageId);
        }
      }

      template<class ConnectionSide>
      void WsProtocol<ConnectionSide>::start()
      {
        resendStoreThread = std::thread([this]() {
          while (!stopped.load())
          {
            std::unique_lock<std::mutex> lck(resendStoreMutex);

            while (resendStore.empty())
            {
              resendStoreCondVar.wait(lck);
            }

            std::list<WsMessageIdentifier> messagesToRemove;
            for (auto& resendStoreItem : resendStore)
            {
              auto messageId = resendStoreItem.first;
              MessageData& messageData = *resendStoreItem.second;

              if (messageData.resendCounter < resendNumber_)
              {
                if (messageData.lastSentTimePoint + std::chrono::seconds(resendTimeout_) <
                    std::chrono::system_clock::now())
                {
                  auto connection = messageData.connection.lock();
                  if (connection)
                  {
                    mdebug_notice("Going to resend message with id: '%d'.", messageId);
                    sendMessageInternal<ConnectionSide>(connection, messageData.message);
                    ++messageData.resendCounter;
                    messageData.lastSentTimePoint = std::chrono::system_clock::now();
                  }
                  else
                  {
                    messagesToRemove.push_back(messageId);
                    mdebug_warn("Going to remove message '%d' from resend store as its client disconnected.",
                                messageId);
                  }
                }
                else
                {
                  continue;
                }
              }
              else
              {
                messagesToRemove.push_back(messageId);
                mdebug_warn("Going to remove message '%d' from resend store as its resend counter gone.", messageId);
              }
            }

            for (auto messageId : messagesToRemove)
            {
              removeFromResendStoreUnsafe(messageId);
            }

            lck.unlock();

            std::this_thread::sleep_for(std::chrono::seconds(resendTimeout_));
          }
        });
      }

      template<class ConnectionSide>
      void WsProtocol<ConnectionSide>::stop()
      {
        bool expected = false;
        if(stopped.compare_exchange_strong(expected, true))
        {
          resendStoreThread.join();
        }
      }

      template<class ConnectionSide>
      WsProtocol<ConnectionSide>::~WsProtocol()
      {
        stop();
      }

      template class WsProtocol<SimpleWeb::SocketServer<SimpleWeb::WS>>;

      template class WsProtocol<SimpleWeb::SocketClient<SimpleWeb::WS>>;

    }

  }

}