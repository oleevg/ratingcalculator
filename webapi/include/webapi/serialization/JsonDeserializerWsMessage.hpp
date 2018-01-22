/*
 * JsonDeserializerWsMessage.hpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_JSONDESERIALIZERWSMESSAGE_HPP
#define RATINGCALCULATOR_JSONDESERIALIZERWSMESSAGE_HPP

#include "../transport/WsMessage.hpp"

#include "JsonDeserializerModel.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace serialization {

      template<>
      struct JsonDeserializer<transport::WsError> {
        static transport::WsError Parse(const boost::property_tree::ptree& tree)
        {
          if (!tree.empty())
          {
            boost::property_tree::ptree child = tree.get_child("error");

            auto id = JsonDeserializer<transport::WsMessageIdentifier>::Parse(tree.get_child("id"));
            auto errorCode = JsonDeserializer<transport::ErrorCode>::Parse(child.get_child("errorCode"));
            auto message = JsonDeserializer<std::string>::Parse(child.get_child("message"));

            return transport::WsError(id, message, errorCode);
          }

          throw SerializerException("Can't deserialize empty JSON value.");
        }
      };

      template<>
      struct JsonDeserializer<transport::WsData> {
        static transport::WsData Parse(const boost::property_tree::ptree& tree)
        {
          if (!tree.empty())
          {

            auto id = JsonDeserializer<transport::WsMessageIdentifier>::Parse(tree.get_child("id"));
            auto data = JsonDeserializer<core::BaseMessage>::Parse(tree.get_child("data"));

            return transport::WsData(id, data);
          }

          throw SerializerException("Can't deserialize empty JSON value.");
        }
      };

      template<>
      struct JsonDeserializer<transport::WsMessage> {
        static transport::WsMessage::Ptr Parse(const boost::property_tree::ptree& tree)
        {
          if (!tree.empty())
          {
            transport::WsMessage::Ptr result;

            auto type = JsonDeserializer<transport::WsMessageType>::Parse(tree.get_child("type"));
            auto id = JsonDeserializer<transport::WsMessageIdentifier>::Parse(tree.get_child("id"));

            if(type == transport::WsMessageType::Ack)
            {
              result = std::make_shared<transport::WsAck>(id);
            }
            else if(type == transport::WsMessageType::Error)
            {
              result = std::make_shared<transport::WsError>(JsonDeserializer<transport::WsError>::Parse(tree));
            }
            else if(type == transport::WsMessageType::Data)
            {
              result = std::make_shared<transport::WsData>(JsonDeserializer<transport::WsData>::Parse(tree));
            }

            return result;
          }

          throw SerializerException("Can't deserialize empty JSON value.");
        }
      };

    }

  }

}

#endif //RATINGCALCULATOR_JSONDESERIALIZERWSMESSAGE_HPP
