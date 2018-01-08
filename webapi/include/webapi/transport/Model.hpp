/*
 * Model.hpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_MODEL_HPP
#define RATINGCALCULATOR_MODEL_HPP

#include <cstdint>
#include <memory>

#include <core/EnumConverter.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      enum class MessageType : uint8_t {
          UserRegistered = 0,
          UserRenamed,
          UserConnected,
          UserDisconnected,
          UserDealWon
      };

      class BaseMessage {
        public:
          typedef std::shared_ptr<BaseMessage> Ptr;

        public:
          BaseMessage(MessageType type);

          MessageType getType() const;

        private:
          MessageType type_;
      };

      template<class T>
      class Message : public BaseMessage {
        public:
          template<typename... Args>
          Message(MessageType type, Args&& ... args): BaseMessage(type), data_(std::forward<Args>(args)...)
          {}

          bool operator==(const Message& message) const
          {
            return ((getType() == message.getType()) && (getData() == message.getData()));
          }

          const T& getData() const
          {
            return data_;
          }

        private:
          T data_;
      };



    }

  }

  namespace core {

    template <>
    EnumConverter<webapi::transport::MessageType>::EnumConverter();

  }

}

#endif //RATINGCALCULATOR_MODEL_HPP
