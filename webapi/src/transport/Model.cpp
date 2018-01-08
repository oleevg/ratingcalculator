/*
 * Model.cpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <webapi/transport/Model.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {


      BaseMessage::BaseMessage(MessageType type):
      type_(type)
      {

      }

      MessageType BaseMessage::getType() const
      {
        return type_;
      }
    }

  }

  namespace core {

    template <>
    EnumConverter<webapi::transport::MessageType>::EnumConverter()
    {
      addTranslation(webapi::transport::MessageType::UserRegistered, "user_registered");
      addTranslation(webapi::transport::MessageType::UserRenamed, "user_renamed");
      addTranslation(webapi::transport::MessageType::UserConnected, "user_connected");
      addTranslation(webapi::transport::MessageType::UserDealWon, "user_deal_won");
      addTranslation(webapi::transport::MessageType::UserDisconnected, "user_disconnected");
    }

  }

}

