/*
 * WsProtocol.hpp
 *
 *  Created on: 1/7/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_WSPROTOCOL_HPP
#define RATINGCALCULATOR_WSPROTOCOL_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <core/ulog.h>

#include <webapi/websockets/server_ws.hpp>
#include <webapi/serialization/JsonSerializerWsMessage.hpp>
#include <webapi/serialization/JsonDeserializerWsMessage.hpp>

#include "Model.hpp"
#include "WsMessage.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      class WsProtocol {
        public:
          WsProtocol(size_t resendNumber, int resendTimeout);

          template <class ConnectionSide>
          void sendMessage(const BaseMessage::Ptr& message, const std::shared_ptr<typename ConnectionSide::Connection>& connection);


          template <class ConnectionSide>
          BaseMessage::Ptr parseMessage(const std::shared_ptr<typename ConnectionSide::Message> message,
                                                  const std::shared_ptr<typename ConnectionSide::Connection>& connection);

        private:
          size_t getInCounter();
          size_t getNextOutCounter();
        private:
          size_t resendNumber_;
          int resendTimeout_;

          size_t inCounter_;
          size_t outCounter_;

      };

      template <class ConnectionSide>
      void WsProtocol::sendMessage(const BaseMessage::Ptr& message, const std::shared_ptr<typename ConnectionSide::Connection>& connection)
      {
        WsData wsData(getNextOutCounter(), message);
        boost::property_tree::ptree tree = serialization::JsonSerializer<decltype(wsData)>::Serialize(wsData);

        auto send_stream = std::make_shared<typename ConnectionSide::SendStream>();

        boost::property_tree::write_json(*send_stream, tree);

        //mdebug_info("Going to send message: '%s'.", stringStream.str().c_str());
        connection->send(send_stream, [](const SimpleWeb::error_code &ec)
        {
          if(ec)
          {
            std::cout << "Server: Error sending message. " <<
                      // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                      "Error: " << ec << ", error message: " << ec.message() << std::endl;
          }
        });
      }

      template <class ConnectionSide>
      BaseMessage::Ptr WsProtocol::parseMessage(const std::shared_ptr<typename ConnectionSide::Message> message,
                                                const std::shared_ptr<typename ConnectionSide::Connection>& connection)
      {
        BaseMessage::Ptr result;

        std::stringstream stringStream;
        stringStream << message->string();

        boost::property_tree::ptree tree;
        boost::property_tree::read_json(stringStream, tree);

        mdebug_info("Received message: '%s'.", stringStream.str().c_str());

        WsMessage::Ptr wsMessage = serialization::JsonDeserializer<WsMessage>::Parse(tree);

        if(wsMessage->getType() == WsMessageType::Ack)
        {
          mdebug_info("Received ACK for message: '%d'.", wsMessage->getId());
        }
        else if(wsMessage->getType() == WsMessageType::Data)
        {
          result = serialization::JsonDeserializer<BaseMessage>::Parse(tree.get_child("data"));
          mdebug_info("Received data message: '%s'.", core::EnumConverter<MessageType>::get_const_instance().toString(result->getType()).c_str());


          WsAck wsAck(wsMessage->getId());
          boost::property_tree::ptree ackTree = serialization::JsonSerializer<WsAck>::Serialize(wsAck);

          auto send_stream = std::make_shared<typename ConnectionSide::SendStream>();

          boost::property_tree::write_json(*send_stream, ackTree);

          mdebug_info("Going to send ACK for: %d.", wsMessage->getId());

          connection->send(send_stream, [](const SimpleWeb::error_code &ec)
          {
            if(ec)
            {
              std::cout << "Server: Error sending message. " <<
                        // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                        "Error: " << ec << ", error message: " << ec.message() << std::endl;
            }
          });
        }

        return result;
      }

    }

  }

}


#endif //RATINGCALCULATOR_WSPROTOCOL_HPP
