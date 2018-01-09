/*
 * JsonSerializerWsMessage.hpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_JSONSERIALIZERWSMESSAGE_HPP
#define RATINGCALCULATOR_JSONSERIALIZERWSMESSAGE_HPP

#include <webapi/transport/WsMessage.hpp>

#include "JsonSerializerModel.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace serialization {

      template <>
      struct JsonSerializer<transport::WsMessage> {
        static boost::property_tree::ptree Serialize(const transport::WsMessage& value)
        {
          boost::property_tree::ptree result;

          result.add_child("id", JsonSerializer<transport::WsMessageIdentifier>::Serialize(value.getId()));
          result.add_child("type", JsonSerializer<transport::WsMessageType>::Serialize(value.getType()));

          return result;
        }
      };

      template <>
      struct JsonSerializer<transport::WsAck> {
        static boost::property_tree::ptree Serialize(const transport::WsAck& value)
        {
          boost::property_tree::ptree result = JsonSerializer<transport::WsMessage>::Serialize(value);

          return result;
        }
      };

      template <>
      struct JsonSerializer<transport::WsError> {
        static boost::property_tree::ptree Serialize(const transport::WsError& value)
        {
          boost::property_tree::ptree result = JsonSerializer<transport::WsMessage>::Serialize(value);

          boost::property_tree::ptree child;
          child.add_child("message", JsonSerializer<std::string>::Serialize(value.getErrorMessage()));
          child.add_child("errorCode", JsonSerializer<transport::ErrorCode>::Serialize(value.getErrorCode()));

          result.add_child("error", child);

          return result;
        }
      };

      template <>
      struct JsonSerializer<transport::WsData> {
        static boost::property_tree::ptree Serialize(const transport::WsData& value)
        {
          boost::property_tree::ptree result = JsonSerializer<transport::WsMessage>::Serialize(value);

          boost::property_tree::ptree child = JsonSerializer<transport::BaseMessage>::Serialize(*value.getData());

          result.add_child("data", child);

          return result;
        }
      };

    }

  }

}

#endif //RATINGCALCULATOR_JSONSERIALIZERWSMESSAGE_HPP
