/*
 * UserRatingWatcher.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <vector>

#include <core/ulog.h>

#include <tempstore/UserRatingProvider.hpp>

#include "UserRatingWatcher.hpp"

namespace rating_calculator {

  namespace service {

    constexpr std::size_t secondsInWeek = 7 * 24 * 3600;

    UserRatingWatcher::UserRatingWatcher(const std::chrono::seconds& ratingUpdateTimeout, std::size_t nRatingPositions,
                                         const core::IDataStoreFactory::Ptr& dataStoreFactory,
                                         const core::ITransportServer::Ptr& transport)
        : ratingUpdateTimeout_(ratingUpdateTimeout), nRatingPositions_(nRatingPositions), transport_(transport),
          stopped_(false), userRatingProvider_(core::TimeHelper::WeekDay::Monday, secondsInWeek, dataStoreFactory)
    {}

    void UserRatingWatcher::start()
    {
      userRatingProvider_.start();

      if (!stopped_.load())
      {
        ratingUpdateThread_ = std::thread(
            [this]()
            {
              while (true)
              {
                std::vector<core::UserIdentifier> snapshot;
                {
                  std::unique_lock<std::mutex> lck(mutex_);
                  cv_.wait(lck,
                           [this]()
                           {
                             return !connectedUsers_.empty() || stopped_.load();
                           });
                  if (stopped_.load())
                  {
                    break;
                  }
                  snapshot.assign(connectedUsers_.begin(), connectedUsers_.end());
                }

                for (core::UserIdentifier userId : snapshot)
                {
                  if (stopped_.load())
                  {
                    break;
                  }
                  sendUserRelativeRating(userId);
                }

                std::unique_lock<std::mutex> lck(mutex_);
                cv_.wait_for(lck, ratingUpdateTimeout_,
                             [this]()
                             {
                               return stopped_.load();
                             });
              }
            });
      }
    }

    void UserRatingWatcher::stop()
    {
      bool expected = false;
      if (stopped_.compare_exchange_strong(expected, true))
      {
        cv_.notify_all();
        userRatingProvider_.stop();
        if (ratingUpdateThread_.joinable())
        {
          ratingUpdateThread_.join();
        }
      }
    }

    void UserRatingWatcher::userConnected(core::UserIdentifier userId)
    {
      {
        std::lock_guard<std::mutex> lck(mutex_);
        if (connectedUsers_.insert(userId).second)
        {
          mdebug_info("New user connected: '%lu'.", userId);
          cv_.notify_one();
        }
        else
        {
          mdebug_warn("Duplicated connection received for user: '%lu'.", userId);
        }
      }
      sendUserRelativeRating(userId);
    }

    void UserRatingWatcher::userDisconnected(core::UserIdentifier userId)
    {
      std::lock_guard<std::mutex> lck(mutex_);

      if (connectedUsers_.erase(userId) == 0)
      {
        mdebug_warn("Disconnection received for unknown user: '%lu'.", userId);
      }
      else
      {
        mdebug_info("User disconnected: '%lu'.", userId);
      }
    }

    void UserRatingWatcher::sendUserRelativeRating(core::UserIdentifier userId)
    {
      mdebug_info("Going to send rating information for user: '%lu'.", userId);

      if (!userRatingProvider_.isUserPresent(userId))
      {
        userRatingProvider_.addDeal(
            core::DealInformation(userId, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), 0.0));
      }

      auto userPosition = userRatingProvider_.getUserPosition(userId);
      auto headPositions = userRatingProvider_.getHeadPositions(nRatingPositions_);
      auto highPositions = userRatingProvider_.getHighPositions(userId, nRatingPositions_);
      auto lowPositions = userRatingProvider_.getLowPositions(userId, nRatingPositions_);

      core::UserRelativeRating userRelativeRating(userPosition, headPositions, highPositions, lowPositions);

      auto message = std::make_shared<core::Message<core::UserRelativeRating>>(core::MessageType::UserRelativeRating,
                                                                               userRelativeRating);
      transport_->sendToUser(userId, message);
    }

  } // namespace service

} // namespace rating_calculator
