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
          MessageType getType() const;

        protected:
          BaseMessage(MessageType type);

        private:
          MessageType type_;
      };

      template<class T>
      class Message : public BaseMessage {
        public:
          template<typename... Args>
          Message(MessageType type, Args&& ... args): BaseMessage(type), data_(std::forward<Args>(args)...)
          {}

          const T& getData() const
          {
            return data_;
          }

        private:
          T data_;
      };

      template <class T>
      bool operator==(const transport::Message<T>& lhs, const transport::Message<T>& rhs)
      {
        bool base = lhs.getType() == rhs.getType();

        bool derived = (lhs.getData() == rhs.getData());

        return (base && derived);
      }

    }

  }

  namespace core {

    template <>
    EnumConverter<webapi::transport::MessageType>::EnumConverter();

  }

}

#endif //RATINGCALCULATOR_MODEL_HPP
