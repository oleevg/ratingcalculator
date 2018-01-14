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
          WsProtocol(size_t resendNumber = 3, int resendTimeout = 3);
          ~WsProtocol();

          void start();

          void stop();

          void sendMessage(const core::BaseMessage::Ptr& message, const std::shared_ptr<typename ConnectionSide::Connection>& connection);


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
          std::mutex resendStoreMutex;

          std::thread resendStoreThread;
          std::condition_variable resendStoreCondVar;
          std::atomic<bool> stopped;
      };

    }

  }

}


#endif //RATINGCALCULATOR_WSPROTOCOL_HPP
