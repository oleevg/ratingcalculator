/*
 * RequestGenerator.cpp
 *
 *  Created on: 1/8/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <random>

#include "RequestGenerator.hpp"

namespace rating_calculator {

  namespace test_client {

    RequestGenerator::RequestGenerator():
    nUsersMax(100)
    {
      users.reserve(nUsersMax);
    }

    webapi::transport::BaseMessage::Ptr RequestGenerator::generateMessage()
    {
      static std::mt19937 rg{std::random_device{}()};
      static std::uniform_int_distribution<size_t> pickMessage(0, 4); // webapi::transport::MessageType

      webapi::transport::BaseMessage::Ptr result;

      uint8_t type = pickMessage(rg);

      if(type == static_cast<uint8_t>(webapi::transport::MessageType::UserConnected))
      {
        size_t nUsers = users.size();

        if(nUsers == 0)
        {
          return result;
        }

        std::uniform_int_distribution<size_t> pickUser(0, nUsers - 1);

        size_t userId = pickUser(rg);

        if(users[userId].isConnected())
        {
          result = std::make_shared<webapi::transport::Message<core::UserIdInformation>>(webapi::transport::MessageType::UserDisconnected, userId);
        }
        else
        {
          result = std::make_shared<webapi::transport::Message<core::UserIdInformation>>(webapi::transport::MessageType::UserConnected, userId);
        }

        users[userId].setConnected(!users[userId].isConnected());
      }
      else if(type == static_cast<uint8_t>(webapi::transport::MessageType::UserRegistered))
      {
        size_t userId = users.size();
        std::string name = "user" + std::to_string(userId);

        core::UserInformation user(userId, name);

        result = std::make_shared<webapi::transport::Message<core::UserInformation>>(webapi::transport::MessageType::UserRegistered, user);

        users.push_back(UserSimulator(userId, name));
      }

    }
  }
  }
