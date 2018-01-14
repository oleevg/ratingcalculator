/*
 * Model.cpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <core/Model.hpp>

namespace rating_calculator {

  namespace core {


      BaseMessage::BaseMessage(MessageType type):
      type_(type)
      {

      }

      MessageType BaseMessage::getType() const
      {
        return type_;
      }

    template <>
    EnumConverter<MessageType>::EnumConverter()
    {
      addConversion(MessageType::UserRegistered, "user_registered");
      addConversion(MessageType::UserRenamed, "user_renamed");
      addConversion(MessageType::UserConnected, "user_connected");
      addConversion(MessageType::UserDealWon, "user_deal_won");
      addConversion(MessageType::UserDisconnected, "user_disconnected");
      addConversion(MessageType::UserRelativeRating, "user_rating");
    }

  }

}

