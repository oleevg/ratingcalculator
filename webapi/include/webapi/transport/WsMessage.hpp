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
#include <core/Model.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      /**
       * @brief Supported protocol messages types.
       */
      enum class WsMessageType {
          Ack = 0,
          Data,
          Error
      };

      typedef uint16_t WsMessageIdentifier;

      /**
       * @brief Base protocol message.
       */
      class WsMessage {
        public:
          typedef std::shared_ptr<WsMessage> Ptr;

        public:
          /**
           * @brief ctor
           * @param type Message type to create.
           * @param id Message identifier.
           */
          WsMessage(WsMessageType type, WsMessageIdentifier id);

          WsMessageType getType() const;
          WsMessageIdentifier getId() const;

        private:
          WsMessageType type_;
          WsMessageIdentifier id_;
      };

      typedef uint8_t ErrorCode;

      /**
       * @brief Message describing internal protocol errors.
       * @detailed Not used so far.
       */
      class WsError : public WsMessage {
        public:
          /**
           * @brief ctor
           * @param id Message identifier.
           * @param errorMessage Error description.
           * @param errorCode Error code.
           */
          WsError(WsMessageIdentifier id, const std::string& errorMessage, ErrorCode errorCode);

          const std::string& getErrorMessage() const;

          ErrorCode getErrorCode() const;

        private:
          std::string errorMessage_;
          ErrorCode errorCode_;
      };

      /**
       * @brief Acknowledgement message to confirm other side message received successfully.
       */
      class WsAck : public WsMessage {
        public:
          /**
           * @brief ctor
           * @param id Other side message identifier to confirm.
           */
          WsAck(WsMessageIdentifier id);
      };

      /**
       * @brief Protocol data message.
       */
      class WsData : public WsMessage {
        public:
          typedef std::shared_ptr<WsData> Ptr;

        public:
          /**
           * @brief ctor
           * @param id Message identifier.
           * @param data Data to send inside protocol data message.
           */
          WsData(WsMessageIdentifier id, const core::BaseMessage::Ptr& data);

          const core::BaseMessage::Ptr& getData() const;

        private:
          core::BaseMessage::Ptr data_;
      };

    }

  }

  namespace core {

    template <>
    EnumConverter<webapi::transport::WsMessageType>::EnumConverter();

  }

}


#endif //RATINGCALCULATOR_WSMESSAGE_HPP
