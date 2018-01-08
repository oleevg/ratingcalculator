/*
 * JsonSerializerModel.hpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_JSONSERIALIZERMODEL_HPP
#define RATINGCALCULATOR_JSONSERIALIZERMODEL_HPP

#include <core/Types.hpp>

#include <webapi/transport/Model.hpp>

#include "JsonSerializer.hpp"

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
      struct JsonSerializer<transport::BaseMessage> {
        static boost::property_tree::ptree Serialize(const transport::BaseMessage& value)
        {
          boost::property_tree::ptree result;

          result.add_child("type", JsonSerializer<decltype(value.getType())>::Serialize(value.getType()));

          return result;
        }
      };

      template<class T>
      struct JsonSerializer<transport::Message<T>> {
        static boost::property_tree::ptree Serialize(const transport::Message<T>& value)
        {
          boost::property_tree::ptree result = JsonSerializer<transport::BaseMessage>::Serialize(value);

          result.add_child("payload", JsonSerializer<T>::Serialize(value.getData()));

          return result;
        }
      };

    }

  }

}


#endif //RATINGCALCULATOR_JSONSERIALIZERMODEL_HPP
