/*
 * JsonDeserializerModel.hpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_JSONDESERIALIZERMODEL_HPP
#define RATINGCALCULATOR_JSONDESERIALIZERMODEL_HPP

#include <boost/format.hpp>

#include <core/Types.hpp>

#include <core/Model.hpp>

#include "JsonDeserializer.hpp"
#include "SerializerException.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace serialization {

      #define THROW_ON_EMPTY_VALUE(ptree) \
        if (value.empty()) \
        { \
          throw SerializerException("Can't deserialize empty JSON value."); \
        }

      template<>
      struct JsonDeserializer<core::UserInformation> {
        static core::UserInformation Parse(const boost::property_tree::ptree& value)
        {
          THROW_ON_EMPTY_VALUE(value)

          auto id = JsonDeserializer<core::UserIdentifier>::Parse(value.get_child("id"));
          auto name = JsonDeserializer<std::string>::Parse(value.get_child("name"));

          return core::UserInformation(id, name);
        }
      };

      template<>
      struct JsonDeserializer<core::DealInformation> {
        static core::DealInformation Parse(const boost::property_tree::ptree& value)
        {
          THROW_ON_EMPTY_VALUE(value)

          auto userId = JsonDeserializer<core::UserIdentifier>::Parse(value.get_child("userId"));
          auto timestamp = JsonDeserializer<uint64_t>::Parse(value.get_child("timestamp"));
          auto amount = JsonDeserializer<float >::Parse(value.get_child("amount"));

          return core::DealInformation(userId, timestamp, amount);
        }
      };

      template<>
      struct JsonDeserializer<core::UserIdInformation> {
        static core::UserIdInformation Parse(const boost::property_tree::ptree& value)
        {
          THROW_ON_EMPTY_VALUE(value)

          auto id = JsonDeserializer<core::UserIdentifier>::Parse(value.get_child("id"));

          return core::UserIdInformation(id);
        }
      };


      template <>
      struct JsonDeserializer<core::UserPosition> {
        static core::UserPosition Parse(const boost::property_tree::ptree& value)
        {
          THROW_ON_EMPTY_VALUE(value)

          auto userInformation = JsonDeserializer<core::UserInformation>::Parse(value.get_child("userInfo"));
          auto position = JsonDeserializer<size_t>::Parse(value.get_child("position"));
          auto amount = JsonDeserializer<float>::Parse(value.get_child("amount"));

          return core::UserPosition(userInformation, position, amount);
        }
      };

      template <>
      struct JsonDeserializer<core::UserRelativeRating> {
        static core::UserRelativeRating Parse(const boost::property_tree::ptree& value)
        {
          THROW_ON_EMPTY_VALUE(value)

          auto userPosition = JsonDeserializer<core::UserPosition>::Parse(value.get_child("userPosition"));
          auto headPositions = JsonDeserializer<core::UserPositionsCollection>::Parse(value.get_child("headPositions"));
          auto highPositions = JsonDeserializer<core::UserPositionsCollection>::Parse(value.get_child("highPositions"));
          auto lowPositions = JsonDeserializer<core::UserPositionsCollection>::Parse(value.get_child("lowPositions"));

          return core::UserRelativeRating(userPosition, headPositions, highPositions, lowPositions);
        }
      };

      template<class T>
      struct JsonDeserializer<core::Message<T>> {
        static core::Message<T> Parse(const boost::property_tree::ptree& value)
        {
          THROW_ON_EMPTY_VALUE(value)

          auto type = JsonDeserializer<core::MessageType>::Parse(value.get_child("type"));

          T payload = JsonDeserializer<T>::Parse(value.get_child("payload"));

          return core::Message<T>(type, payload);
        }
      };

      template<>
      struct JsonDeserializer<core::BaseMessage> {
        static core::BaseMessage::Ptr Parse(const boost::property_tree::ptree& value)
        {
          THROW_ON_EMPTY_VALUE(value)

          core::BaseMessage::Ptr result;

          auto type = JsonDeserializer<core::MessageType>::Parse(value.get_child("type"));

          if(type == core::MessageType::UserRegistered)
          {
            result = std::make_shared<core::Message<core::UserInformation>>(JsonDeserializer<core::Message<core::UserInformation>>::Parse(value));
          }
          else if(type == core::MessageType::UserRenamed)
          {
            result = std::make_shared<core::Message<core::UserInformation>>(JsonDeserializer<core::Message<core::UserInformation>>::Parse(value));
          }
          else if(type == core::MessageType::UserConnected)
          {
            result = std::make_shared<core::Message<core::UserIdInformation>>(JsonDeserializer<core::Message<core::UserIdInformation>>::Parse(value));
          }
          else if(type == core::MessageType::UserDisconnected)
          {
            result = std::make_shared<core::Message<core::UserIdInformation>>(JsonDeserializer<core::Message<core::UserIdInformation>>::Parse(value));
          }
          else if(type == core::MessageType::UserDealWon)
          {
            result = std::make_shared<core::Message<core::DealInformation>>(JsonDeserializer<core::Message<core::DealInformation>>::Parse(value));
          }
          else if(type == core::MessageType::UserRelativeRating)
          {
            result = std::make_shared<core::Message<core::UserRelativeRating>>(JsonDeserializer<core::Message<core::UserRelativeRating>>::Parse(value));
          }
          else
          {
            const auto& enumConverter = core::EnumConverter<core::MessageType>::get_const_instance();
            throw SerializerException((boost::format("Can't deserialize unsupported message '%s'.") % enumConverter.toString(type).c_str()).str());
          }

          return result;
        }
      };

    }

  }

}

#endif //RATINGCALCULATOR_JSONDESERIALIZERMODEL_HPP
