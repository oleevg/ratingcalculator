/*
 * JsonSerializerModel.hpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_JSONSERIALIZERMODEL_HPP
#define RATINGCALCULATOR_JSONSERIALIZERMODEL_HPP

#include <boost/format.hpp>

#include <core/Types.hpp>

#include <core/Model.hpp>

#include "JsonSerializer.hpp"
#include "SerializerException.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace serialization {

      template<>
      struct JsonSerializer<core::UserInformation> {
        static boost::property_tree::ptree Serialize(const core::UserInformation& value)
        {
          boost::property_tree::ptree result;

          result.add_child("id", JsonSerializer<core::UserIdentifier>::Serialize(value.id));
          result.add_child("name", JsonSerializer<std::string>::Serialize(value.name));

          return result;
        }
      };

      template<>
      struct JsonSerializer<core::DealInformation> {
        static boost::property_tree::ptree Serialize(const core::DealInformation& value)
        {
          boost::property_tree::ptree result;

          result.add_child("userId", JsonSerializer<decltype(value.userId)>::Serialize(value.userId));
          result.add_child("timestamp", JsonSerializer<decltype(value.timestamp)>::Serialize(value.timestamp));
          result.add_child("amount", JsonSerializer<decltype(value.amount)>::Serialize(value.amount));

          return result;
        }
      };

      template<>
      struct JsonSerializer<core::UserIdInformation> {
        static boost::property_tree::ptree Serialize(const core::UserIdInformation& value)
        {
          boost::property_tree::ptree result;

          result.add_child("id", JsonSerializer<decltype(value.id)>::Serialize(value.id));

          return result;
        }
      };

      template<>
      struct JsonSerializer<core::UserPosition> {
        static boost::property_tree::ptree Serialize(const core::UserPosition& value)
        {
          boost::property_tree::ptree result;

          result.add_child("userInfo", JsonSerializer<decltype(value.userInformation)>::Serialize(value.userInformation));
          result.add_child("position", JsonSerializer<decltype(value.position)>::Serialize(value.position));
          result.add_child("amount", JsonSerializer<decltype(value.amount)>::Serialize(value.amount));

          return result;
        }
      };

      template<>
      struct JsonSerializer<core::UserRelativeRating> {
        static boost::property_tree::ptree Serialize(const core::UserRelativeRating& value)
        {
          boost::property_tree::ptree result;

          result.add_child("userPosition", JsonSerializer<decltype(value.userPosition)>::Serialize(value.userPosition));
          result.add_child("headPositions", JsonSerializer<decltype(value.headPositions)>::Serialize(value.headPositions));
          result.add_child("highPositions", JsonSerializer<decltype(value.highPositions)>::Serialize(value.highPositions));
          result.add_child("lowPositions", JsonSerializer<decltype(value.lowPositions)>::Serialize(value.lowPositions));

          return result;
        }
      };

      template<class T>
      struct JsonSerializer<core::Message<T>> {
        static boost::property_tree::ptree Serialize(const core::Message<T>& value)
        {
          boost::property_tree::ptree result;
          result.add_child("type", JsonSerializer<decltype(value.getType())>::Serialize(value.getType()));
          result.add_child("payload", JsonSerializer<T>::Serialize(value.getData()));

          return result;
        }
      };

      template<>
      struct JsonSerializer<core::BaseMessage> {
        static boost::property_tree::ptree Serialize(const core::BaseMessage& value)
        {
          boost::property_tree::ptree result;

          if(value.getType() == core::MessageType::UserRegistered)
          {
            auto userRegisteredMessage = static_cast<const core::Message<core::UserInformation>&>(value);
            result = JsonSerializer<core::Message<core::UserInformation>>::Serialize(userRegisteredMessage);
          }
          else if(value.getType() == core::MessageType::UserRenamed)
          {
            auto userRenamedMessage = static_cast<const core::Message<core::UserInformation>&>(value);
            result = JsonSerializer<core::Message<core::UserInformation>>::Serialize(userRenamedMessage);
          }
          else if(value.getType() == core::MessageType::UserConnected)
          {
            auto userConnectedMessage = static_cast<const core::Message<core::UserIdInformation>&>(value);
            result = JsonSerializer<core::Message<core::UserIdInformation>>::Serialize(userConnectedMessage);
          }
          else if(value.getType() == core::MessageType::UserDisconnected)
          {
            auto userDisconnectedMessage = static_cast<const core::Message<core::UserIdInformation>&>(value);
            result = JsonSerializer<core::Message<core::UserIdInformation>>::Serialize(userDisconnectedMessage);
          }
          else if(value.getType() == core::MessageType::UserDealWon)
          {
            auto userDealWonMessage = static_cast<const core::Message<core::DealInformation>&>(value);
            result = JsonSerializer<core::Message<core::DealInformation>>::Serialize(userDealWonMessage);
          }
          else if(value.getType() == core::MessageType::UserRelativeRating)
          {
            auto userRelativeRatingWonMessage = static_cast<const core::Message<core::UserRelativeRating>&>(value);
            result = JsonSerializer<core::Message<core::UserRelativeRating>>::Serialize(userRelativeRatingWonMessage);
          }
          else
          {
            const auto& enumConverter = core::EnumConverter<core::MessageType>::get_const_instance();
            throw SerializerException((boost::format("Can't serialize unsupported message '%s'.") % enumConverter.toString(value.getType()).c_str()).str());
          }

          return result;
        }
      };

    }

  }

}


#endif //RATINGCALCULATOR_JSONSERIALIZERMODEL_HPP
