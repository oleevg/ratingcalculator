/*
 * WsMessage.cpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <webapi/transport/WsMessage.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      WsMessage::WsMessage(WsMessageType type, WsMessageIdentifier id) :
              type_(type), id_(id)
      {

      }

      WsMessageType WsMessage::getType() const
      {
        return type_;
      }

      WsMessageIdentifier WsMessage::getId() const
      {
        return id_;
      }

      WsError::WsError(WsMessageIdentifier id, const std::string& errorMessage, ErrorCode errorCode)
              : WsMessage(WsMessageType::Error, id), errorMessage_(errorMessage), errorCode_(errorCode)
      {}

      const std::string& WsError::getErrorMessage() const
      {
        return errorMessage_;
      }

      ErrorCode WsError::getErrorCode() const
      {
        return errorCode_;
      }

      WsAck::WsAck(WsMessageIdentifier id) : WsMessage(WsMessageType::Ack, id)
      {}

    }

  }

  namespace core {

    template <>
    EnumConverter<webapi::transport::WsMessageType>::EnumConverter()
    {
      addTranslation(webapi::transport::WsMessageType::Data, "data");
      addTranslation(webapi::transport::WsMessageType::Ack, "ack");
      addTranslation(webapi::transport::WsMessageType::Error, "error");
    }

  }

}