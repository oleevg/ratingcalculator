/*
 * WsProtocol.hpp
 *
 *  Created on: 1/7/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_WSPROTOCOL_HPP
#define RATINGCALCULATOR_WSPROTOCOL_HPP

#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <core/Model.hpp>
#include <core/ulog.h>

#include <webapi/serialization/JsonSerializerWsMessage.hpp>
#include <webapi/serialization/JsonDeserializerWsMessage.hpp>

#include "WsMessage.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      /**
       * @brief Class describing application protocol used above WebSocket communication.
       * @tparam ConnectionSide Server or client type going from SimpleWebSocket project.
       */
      template <class ConnectionSide>
      class WsProtocol {
        public:
          typedef std::shared_ptr<WsProtocol> Ptr;

        private:
            struct MessageData {
            public:
              typedef std::shared_ptr<MessageData> Ptr;

            public:
              MessageData(const std::shared_ptr<typename ConnectionSide::Connection>& connection, const WsData& message)
                      : connection(connection), message(message), resendCounter(0), lastSentTimePoint(std::chrono::system_clock::now())
              {}


              WsData message;
              size_t resendCounter;
              std::weak_ptr<typename ConnectionSide::Connection> connection;
              std::chrono::system_clock::time_point lastSentTimePoint;
          };

          typedef std::unordered_map<WsMessageIdentifier, typename MessageData::Ptr> ResendMessageStore;

        public:
          /**
           * @brief ctor
           * @param resendNumber Number of attempts to send a packet if it is not acknowledged.
           * @param resendTimeout Timeout between resend attempts.
           */
          WsProtocol(size_t resendNumber = 3, int resendTimeout = 3);
          ~WsProtocol();

          /**
           * @brief Starts the protocol thread that handles resend messages queue.
           */
          void start();

          /**
           * @brief Stops the resend messages queue thread.
           */
          void stop();

          /**
           * @brief Sends the protocol message to the client.
           * @param message Message to be send.
           * @param connection Client' connection to send message to.
           */
          void sendMessage(const core::BaseMessage::Ptr& message, const std::shared_ptr<typename ConnectionSide::Connection>& connection);


          /**
           * @brief Parses received data to obtain protocol internal message.
           * @param message Received message form WebSocket layer.
           * @param connection Client's connection.
           * @return Custom message or nullptr if protocol service message was received.
           */
          core::BaseMessage::Ptr parseMessage(const std::shared_ptr<typename ConnectionSide::Message> message,
                                                  const std::shared_ptr<typename ConnectionSide::Connection>& connection);

        private:
          size_t getInCounter();
          size_t getNextOutCounter();

          void addToResendStore(const std::shared_ptr<typename ConnectionSide::Connection>& connection, const typename MessageData::Ptr& messageData);

          void removeFromResendStore(WsMessageIdentifier messageId);
          void removeFromResendStoreUnsafe(WsMessageIdentifier messageId);

        private:
          size_t resendNumber_;
          int resendTimeout_;

          size_t inCounter_;
          size_t outCounter_;

          ResendMessageStore resendStore;

          std::thread resendStoreThread;
          std::mutex resendStoreMutex;
          std::condition_variable resendStoreCondVar;

          std::atomic<bool> stopped;
          std::mutex stopMutex;
          std::condition_variable stopCondVar;
      };

    }

  }

}


#endif //RATINGCALCULATOR_WSPROTOCOL_HPP
