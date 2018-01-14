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

    const size_t secondsInDay = 24*3600;

    UserRatingWatcher::UserConnection::UserConnection(const core::UserIdentifier& _userIdentifier,
                                                      const std::shared_ptr<WsConnection>& _connection) :
            userIdentifier(_userIdentifier), connection(_connection)
    {}

    UserRatingWatcher::UserRatingWatcher(const core::IDataStoreFactory::Ptr& dataStoreFactory, int ratingUpdateTimeout,
                                         size_t nRatingPositions,
                                         const webapi::transport::WsProtocol<WsServer>::Ptr& protocol)
            : dataStoreFactory_(dataStoreFactory), ratingUpdateTimeout_(ratingUpdateTimeout),
              nRatingPositions_(nRatingPositions), protocol_(protocol), stopped(false),
              sortedDealStore(dataStoreFactory, core::TimeHelper::WeekDay::Monday, secondsInDay)
    {
      ratingUpdateThread = std::thread([this]()
                                       {
                                         while(!stopped.load())
                                         {
                                           std::unique_lock<std::mutex> lck(userConnectionsMutex);
                                           while (userConnections.empty())
                                           {
                                             userConnectionsCondVar.wait(lck);
                                           }

                                           for (const auto& item : userConnections)
                                           {
                                             auto& userConnection = item.second;
                                             if(userConnection->connected.load())
                                             {
                                               auto connection = userConnection->connection.lock();
                                               if(!connection)
                                               {
                                                 mdebug_error("Found connected user '%d' with unspecified connection.", userConnection->userIdentifier);
                                                 continue;
                                               }

                                               auto userPosition = sortedDealStore.getUserPosition(userConnection->userIdentifier);
                                               auto headPositions = sortedDealStore.getHeadPositions(nRatingPositions_);
                                               auto highPositions = sortedDealStore.getHighPositions(nRatingPositions_,
                                                                                                     userConnection->userIdentifier);
                                               auto lowPositions = sortedDealStore.getLowPositions(nRatingPositions_,
                                                                                                   userConnection->userIdentifier);

                                               core::UserRelativeRating userRelativeRating(userPosition, headPositions, highPositions, lowPositions);

                                               auto message = std::make_shared<core::Message<core::UserRelativeRating>>(core::MessageType::UserRelativeRating, userRelativeRating);
                                               protocol_->sendMessage(message, connection);
                                             }
                                           }

                                           lck.unlock();
                                           std::this_thread::sleep_for(std::chrono::seconds(ratingUpdateTimeout_));
                                         }
                                       });

    }

    void UserRatingWatcher::userConnected(const core::UserIdentifier& userIdentifier, const std::shared_ptr<WsConnection>& connection)
    {
      auto iter = userConnections.find(userIdentifier);
      if(iter == userConnections.end())
      {
        mdebug_info("User connected first time: %d.", userIdentifier);

        userConnections.insert({userIdentifier, std::make_shared<UserConnection>(userIdentifier, connection)});
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
    }

    void UserRatingWatcher::userDisconnected(const core::UserIdentifier& userIdentifier)
    {
      auto iter = userConnections.find(userIdentifier);
      if(iter == userConnections.end())
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
        //iter->second->connection.store(nullptr);
      }
    }

    void UserRatingWatcher::stop()
    {
      bool expected = false;
      if(stopped.compare_exchange_strong(expected, true))
      {
        sortedDealStore.stop();
        ratingUpdateThread.join();
      }
    }

  }
}
