/*
 * WsMessage.hpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_WSMESSAGE_HPP
#define RATINGCALCULATOR_WSMESSAGE_HPP

#include <cstdint>
#include <string>
#include <memory>

#include <core/EnumConverter.hpp>
#include "Model.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      enum class WsMessageType {
          Ack = 0,
          Data,
          Error
      };

      typedef uint16_t WsMessageIdentifier;

      class WsMessage {
        public:
          typedef std::shared_ptr<WsMessage> Ptr;

        public:
          WsMessage(WsMessageType type, WsMessageIdentifier id);

          WsMessageType getType() const;
          WsMessageIdentifier getId() const;

        private:
          WsMessageType type_;
          WsMessageIdentifier id_;
      };

      typedef uint8_t ErrorCode;

      class WsError : public WsMessage {
        public:
          WsError(WsMessageIdentifier id, const std::string& errorMessage, ErrorCode errorCode);

          const std::string& getErrorMessage() const;

          ErrorCode getErrorCode() const;

        private:
          std::string errorMessage_;
          ErrorCode errorCode_;
      };

      class WsAck : public WsMessage {
        public:
          WsAck(WsMessageIdentifier id);
      };

        class WsData : public WsMessage {
        public:
          typedef std::shared_ptr<WsData> Ptr;

        public:
         WsData(WsMessageIdentifier id, const BaseMessage::Ptr& data);

          const BaseMessage::Ptr& getData() const;

        private:
            BaseMessage::Ptr data_;
      };

    }

  }

  namespace core {

    template <>
    EnumConverter<webapi::transport::WsMessageType>::EnumConverter();

  }

}


#endif //RATINGCALCULATOR_WSMESSAGE_HPP
