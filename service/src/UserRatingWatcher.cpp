/*
 * UserRatingWatcher.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <core/ulog.h>

#include <tempstore/SortedUserDealStore.hpp>

#include "UserRatingWatcher.hpp"

namespace rating_calculator {

  namespace service {

    const size_t secondsInWeek = 7*24*3600;
    const size_t testPeriod = 30;

    UserRatingWatcher::UserConnection::UserConnection(const core::UserIdentifier& _userIdentifier,
                                                      const std::shared_ptr<WsConnection>& _connection) :
            userIdentifier(_userIdentifier), connection(_connection)
    {}

    UserRatingWatcher::UserRatingWatcher(int ratingUpdateTimeout, size_t nRatingPositions,
                                         const core::IDataStoreFactory::Ptr& dataStoreFactory,
                                         const webapi::transport::WsProtocol<WsServer>::Ptr& protocol)
            : ratingUpdateTimeout_(ratingUpdateTimeout), nRatingPositions_(nRatingPositions),
              dataStoreFactory_(dataStoreFactory),
              protocol_(protocol), stopped_(false),
              sortedDealStore_(core::TimeHelper::WeekDay::Monday, secondsInWeek, dataStoreFactory)
    {}

    void UserRatingWatcher::start()
    {
      sortedDealStore_.start();

      if(!stopped_.load())
      {
        ratingUpdateThread_ = std::thread([this]() {
          while (!stopped_.load())
          {
            mdebug_info("Going to update users ratings.");
            std::unique_lock<std::mutex> lck(userConnectionsMutex_);
            while (userConnections_.empty())
            {
              userConnectionsCondVar_.wait(lck);
            }

            mdebug_info("UserConnections.size()=%d.", userConnections_.size());
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
            std::this_thread::sleep_for(std::chrono::seconds(ratingUpdateTimeout_));
          }
        });
      }
    }

    void UserRatingWatcher::stop()
    {
      bool expected = false;
      if(stopped_.compare_exchange_strong(expected, true))
      {
        sortedDealStore_.stop();
        ratingUpdateThread_.join();
      }
    }

    void UserRatingWatcher::userConnected(const core::UserIdentifier& userIdentifier, const std::shared_ptr<WsConnection>& connection)
    {
      std::lock_guard<std::mutex> lck(userConnectionsMutex_);

      auto iter = userConnections_.find(userIdentifier);
      if(iter == userConnections_.end())
      {
        mdebug_info("New user connected: %d.", userIdentifier);

        auto userConnection = std::make_shared<UserConnection>(userIdentifier, connection);
        iter = userConnections_.insert({userIdentifier, userConnection}).first;

        userConnectionsCondVar_.notify_one();
      }
      else
      {
        if(iter->second->connected.load())
        {
          mdebug_warn("Duplicated connection received for user: %d.", userIdentifier);
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
        mdebug_warn("Not connected user disconnected: %d.", userIdentifier);
      }
      else
      {
        if(!iter->second->connected.load())
        {
          mdebug_warn("Duplicated disconnection received for user: %d.", userIdentifier);
        }

        iter->second->connected.store(false);
        iter->second->connection.reset();
      }
    }

    void UserRatingWatcher::sendUserRelativeRating(const core::UserIdentifier& userIdentifier,
                                                   const std::weak_ptr<WsConnection>& connection) const
    {
      auto userConnection = connection.lock();
      if(!userConnection)
      {
        mdebug_warn("Can't send user's rating as its connection gone.");
        return;
      }

      mdebug_info("Going to send rating information for user: %d.", userIdentifier);

      auto userPosition = sortedDealStore_.getUserPosition(userIdentifier);
      auto headPositions = sortedDealStore_.getHeadPositions(nRatingPositions_);
      auto highPositions = sortedDealStore_.getHighPositions(nRatingPositions_, userIdentifier);
      auto lowPositions = sortedDealStore_.getLowPositions(nRatingPositions_, userIdentifier);

      core::UserRelativeRating userRelativeRating (userPosition, headPositions, highPositions, lowPositions);

      auto message = std::make_shared<core::Message<core::UserRelativeRating>>(
              core::MessageType::UserRelativeRating, userRelativeRating);
      protocol_->sendMessage(message, userConnection);
    }

  }
}
