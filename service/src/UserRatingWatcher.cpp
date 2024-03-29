/*
 * UserRatingWatcher.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <core/ulog.h>

#include <tempstore/UserRatingProvider.hpp>

#include "UserRatingWatcher.hpp"

namespace rating_calculator {

  namespace service {

    const size_t secondsInWeek = 7*24*3600;

    UserRatingWatcher::UserConnection::UserConnection(const core::UserIdentifier& _userIdentifier,
                                                      const std::shared_ptr<WsConnection>& _connection) :
            userIdentifier(_userIdentifier), connection(_connection), connected(true)
    {}

    UserRatingWatcher::UserRatingWatcher(int ratingUpdateTimeout, size_t nRatingPositions,
                                         const core::IDataStoreFactory::Ptr& dataStoreFactory,
                                         const webapi::transport::WsProtocol<WsServer>::Ptr& protocol)
            : ratingUpdateTimeout_(ratingUpdateTimeout), nRatingPositions_(nRatingPositions),
              dataStoreFactory_(dataStoreFactory),
              protocol_(protocol), stopped_(false),
              userRatingProvider_(core::TimeHelper::WeekDay::Monday, secondsInWeek, dataStoreFactory)
    {}

    void UserRatingWatcher::start()
    {
      userRatingProvider_.start();

      if(!stopped_.load())
      {
        ratingUpdateThread_ = std::thread([this]() {
          while (!stopped_.load())
          {
            std::unique_lock<std::mutex> lck(userConnectionsMutex_);
            while (userConnections_.empty() && !stopped_.load())
            {
              userConnectionsCondVar_.wait_for(lck, std::chrono::seconds(1));
            }

            for (const auto& item : userConnections_)
            {
              auto& userConnection = item.second;
              if (userConnection->connected.load())
              {
                mdebug_info("User '%d' connected.", userConnection->userIdentifier);
                auto connection = userConnection->connection.lock();
                if (!connection)
                {
                  mdebug_error("Found connected user '%d' with unspecified connection.",
                               userConnection->userIdentifier);
                  continue;
                }

                sendUserRelativeRating(userConnection->userIdentifier, userConnection->connection);
              }
            }

            lck.unlock();

            if(!stopped_.load())
            {
              std::unique_lock<std::mutex> stopLock(stopMutex_);
              stopCondVar_.wait_for(stopLock, std::chrono::seconds(ratingUpdateTimeout_));
            }
          }
        });
      }
    }

    void UserRatingWatcher::stop()
    {
      bool expected = false;
      if(stopped_.compare_exchange_strong(expected, true))
      {
        {
          std::lock_guard<std::mutex> stopLock(stopMutex_);
          stopCondVar_.notify_one();
        }

        userRatingProvider_.stop();
        ratingUpdateThread_.join();
      }
    }

    void UserRatingWatcher::userConnected(const core::UserIdentifier& userIdentifier, const std::shared_ptr<WsConnection>& connection)
    {
      std::lock_guard<std::mutex> lck(userConnectionsMutex_);

      auto iter = userConnections_.find(userIdentifier);
      if(iter == userConnections_.end())
      {
        mdebug_info("New user connected: '%d'.", userIdentifier);

        auto userConnection = std::make_shared<UserConnection>(userIdentifier, connection);
        iter = userConnections_.insert({userIdentifier, userConnection}).first;

        userConnectionsCondVar_.notify_one();
      }
      else
      {
        if(iter->second->connected.load())
        {
          mdebug_warn("Duplicated connection received for user: '%d'.", userIdentifier);
        }

        iter->second->connected.store(true);
        iter->second->connection = connection;
      }

      sendUserRelativeRating(userIdentifier, iter->second->connection);
    }

    void UserRatingWatcher::userDisconnected(const core::UserIdentifier& userIdentifier)
    {
      std::lock_guard<std::mutex> lck(userConnectionsMutex_);

      auto iter = userConnections_.find(userIdentifier);
      if(iter == userConnections_.end())
      {
        mdebug_warn("Not connected user disconnected: '%d'.", userIdentifier);
      }
      else
      {
        if(!iter->second->connected.load())
        {
          mdebug_warn("Duplicated disconnection received for user: '%d'.", userIdentifier);
        }

        iter->second->connected.store(false);
        iter->second->connection.reset();
      }
    }

    void UserRatingWatcher::sendUserRelativeRating(const core::UserIdentifier& userIdentifier,
                                                   const std::weak_ptr<WsConnection>& connection)
    {
      auto userConnection = connection.lock();
      if(!userConnection)
      {
        mdebug_warn("Can't send user's rating as its connection has gone.");
        return;
      }

      mdebug_info("Going to send rating information for user: '%d'.", userIdentifier);

      if(!userRatingProvider_.isUserPresent(userIdentifier))
      {
        // TODO: eliminate necessity to register user with 0.0 deal before his/her real deals information received.
        userRatingProvider_.addDeal(core::DealInformation(userIdentifier, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), 0.0));
      }

      auto userPosition = userRatingProvider_.getUserPosition(userIdentifier);
      auto headPositions = userRatingProvider_.getHeadPositions(nRatingPositions_);
      auto highPositions = userRatingProvider_.getHighPositions(userIdentifier, nRatingPositions_);
      auto lowPositions = userRatingProvider_.getLowPositions(userIdentifier, nRatingPositions_);

      core::UserRelativeRating userRelativeRating (userPosition, headPositions, highPositions, lowPositions);

      auto message = std::make_shared<core::Message<core::UserRelativeRating>>(
              core::MessageType::UserRelativeRating, userRelativeRating);
      protocol_->sendMessage(message, userConnection);
    }

  }
}
