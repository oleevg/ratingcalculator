/*
 * RequestGenerator.cpp
 *
 *  Created on: 1/8/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <random>
#include <chrono>

#include "RequestGenerator.hpp"

namespace rating_calculator {

  namespace test_client {

    RequestGenerator::RequestGenerator(std::size_t nUsers) : nUsersMax(nUsers)
    {
      users.reserve(nUsersMax);
    }

    core::BaseMessage::Ptr RequestGenerator::generateUserCommonMessage()
    {
      static std::mt19937 rg{std::random_device{}()};
      static std::uniform_int_distribution<std::size_t> pickMessage(
          static_cast<std::uint8_t>(core::MessageType::UserRenamed),
          static_cast<std::uint8_t>(core::MessageType::UserDealWon));

      core::BaseMessage::Ptr result;

      std::uint8_t type = pickMessage(rg);

      if (type == static_cast<std::uint8_t>(core::MessageType::UserConnected) ||
          type == static_cast<std::uint8_t>(core::MessageType::UserDisconnected))
      {
        std::size_t nUsers = getRegisteredUsersNumber();

        std::uniform_int_distribution<std::size_t> pickUser(0, nUsers - 1);

        std::size_t userId = pickUser(rg);

        if (users[userId].isConnected())
        {
          result =
              std::make_shared<core::Message<core::UserIdInformation>>(core::MessageType::UserDisconnected, userId);
        }
        else
        {
          result = std::make_shared<core::Message<core::UserIdInformation>>(core::MessageType::UserConnected, userId);
        }

        users[userId].setConnected(!users[userId].isConnected());
      }
      else if (type == static_cast<std::uint8_t>(core::MessageType::UserRenamed))
      {
        std::size_t nUsers = getRegisteredUsersNumber();

        std::uniform_int_distribution<std::size_t> pickUser(0, nUsers - 1);

        std::size_t userId = pickUser(rg);

        auto& user = users[userId];
        user.changeName(user.getName() + "_");

        result = std::make_shared<core::Message<core::UserInformation>>(core::MessageType::UserRenamed,
                                                                        core::UserInformation(userId, user.getName()));
      }
      else if (type == static_cast<std::uint8_t>(core::MessageType::UserDealWon))
      {
        std::size_t nUsers = getRegisteredUsersNumber();

        std::uniform_int_distribution<std::size_t> pickUser(0, nUsers - 1);
        std::uniform_real_distribution<float> pickAmount(0.0, 100.0);

        std::size_t userId = pickUser(rg);
        float amount = pickAmount(rg);
        auto timeStamp = std::chrono::system_clock::now();

        result = std::make_shared<core::Message<core::DealInformation>>(
            core::MessageType::UserDealWon, userId, std::chrono::system_clock::to_time_t(timeStamp), amount);
      }

      return result;
    }

    core::BaseMessage::Ptr RequestGenerator::generateUserRegisteredMessage()
    {
      std::lock_guard<std::mutex> lck(usersMutex);

      std::size_t userId = users.size();
      std::string name = "user" + std::to_string(userId);

      core::UserInformation user(userId, name);

      users.emplace_back(userId, name);

      // Otherwise we notify before message reaches recipient
      if (userId > 3)
      {
        usersCondVar.notify_one();
      }

      return std::make_shared<core::Message<core::UserInformation>>(core::MessageType::UserRegistered, user);
    }

    void RequestGenerator::waitForUsersToRegister()
    {
      std::unique_lock<std::mutex> lck(usersMutex);
      usersCondVar.wait(lck,
                        [this]()
                        {
                          return !users.empty();
                        });
    }

    std::size_t RequestGenerator::getRegisteredUsersNumber() const
    {
      std::lock_guard<std::mutex> lck(usersMutex);

      return users.size();
    }
  } // namespace test_client
} // namespace rating_calculator
