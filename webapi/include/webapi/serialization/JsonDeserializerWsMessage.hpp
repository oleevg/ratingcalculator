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

//      template<>
//      struct JsonDeserializer<transport::WsAck> {
//        static transport::WsAck Parse(const boost::property_tree::ptree& tree)
//        {
//          if (!tree.empty())
//          {
//            transport::WsMessage wsMessage = JsonDeserializer<transport::WsMessage>::Parse(tree);
//
//            return transport::WsAck(wsMessage.getId());
//          }
//
//          throw SerializerException("Can't deserialize empty JSON value.");
//        }
//      };

      template<class T>
      struct JsonDeserializer<transport::WsData<T>> {
        static transport::WsData<T> Parse(const boost::property_tree::ptree& tree)
        {
          if (!tree.empty())
          {

            auto id = JsonDeserializer<transport::WsMessageIdentifier>::Parse(tree.get_child("id"));
            std::shared_ptr<T> data = std::make_shared<T>(JsonDeserializer<T>::Parse(tree.get_child("data")));

            return transport::WsData<T>(id, *data);
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
              transport::BaseMessage::Ptr baseMessage = JsonDeserializer<transport::BaseMessage>::Parse(tree.get_child("data"));

              if(baseMessage->getType() == transport::MessageType::UserConnected)
              {
                auto userConnectedMessage = std::static_pointer_cast<transport::Message<core::UserIdInformation>>(baseMessage);
                result = std::make_shared<transport::WsData<core::UserIdInformation>>(id, userConnectedMessage->getData());
              }
              else if(baseMessage->getType() == transport::MessageType::UserRegistered)
              {
                auto userRegisteredMessage = std::static_pointer_cast<transport::Message<core::UserInformation>>(baseMessage);
                result = std::make_shared<transport::WsData<core::UserInformation>>(id, userRegisteredMessage->getData());
              }
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
