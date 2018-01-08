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

#include <webapi/transport/Model.hpp>

#include "JsonDeserializer.hpp"
#include "SerializerException.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace serialization {

      template<>
      struct JsonDeserializer<core::UserInformation> {
        static core::UserInformation Parse(const boost::property_tree::ptree& value)
        {
          if (!value.empty())
          {
            auto id = JsonDeserializer<core::UserIdentifier>::Parse(value.get_child("id"));
            auto name = JsonDeserializer<std::string>::Parse(value.get_child("name"));

            return core::UserInformation(id, name);
          }

          throw SerializerException("Can't deserialize empty JSON value.");
        }
      };

      template<>
      struct JsonDeserializer<core::DealInformation> {
        static core::DealInformation Parse(const boost::property_tree::ptree& value)
        {
          if (!value.empty())
          {
            auto userId = JsonDeserializer<core::UserIdentifier>::Parse(value.get_child("userId"));
            auto timestamp = JsonDeserializer<uint64_t>::Parse(value.get_child("timestamp"));
            auto amount = JsonDeserializer<float >::Parse(value.get_child("amount"));

            return core::DealInformation(userId, timestamp, amount);
          }

          throw SerializerException("Can't deserialize empty JSON value.");
        }
      };

      template<>
      struct JsonDeserializer<core::UserIdInformation> {
        static core::UserIdInformation Parse(const boost::property_tree::ptree& value)
        {
          if (!value.empty())
          {
            auto id = JsonDeserializer<core::UserIdentifier>::Parse(value.get_child("id"));

            return core::UserIdInformation(id);
          }

          throw SerializerException("Can't deserialize empty JSON value.");
        }
      };


      template<class T>
      struct JsonDeserializer<transport::Message<T>> {
        static transport::Message<T> Parse(const boost::property_tree::ptree& value)
        {
          if (!value.empty())
          {
            auto type = JsonDeserializer<transport::MessageType>::Parse(value.get_child("type"));

            T payload = JsonDeserializer<T>::Parse(value.get_child("payload"));

            return transport::Message<T>(type, payload);
          }

          throw SerializerException("Can't deserialize empty JSON value.");
        }
      };

      template<>
      struct JsonDeserializer<transport::BaseMessage> {
        static transport::BaseMessage::Ptr Parse(const boost::property_tree::ptree& value)
        {
          if (!value.empty())
          {
            transport::BaseMessage::Ptr result;

            auto type = JsonDeserializer<transport::MessageType>::Parse(value.get_child("type"));

            if(type == transport::MessageType::UserRegistered)
            {
              result = std::make_shared<transport::Message<core::UserInformation>>(JsonDeserializer<transport::Message<core::UserInformation>>::Parse(value));
            }
            else if(type == transport::MessageType::UserConnected)
            {
              result = std::make_shared<transport::Message<core::UserIdInformation>>(JsonDeserializer<transport::Message<core::UserIdInformation>>::Parse(value));
            }

            return result;
          }

          throw SerializerException("Can't deserialize empty JSON value.");
        }
      };

    }

  }

}

#endif //RATINGCALCULATOR_JSONDESERIALIZERMODEL_HPP
