/*
 * UserRatingWatcher.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <semaphore>
#include <stop_token>
#include <vector>

#include <core/ulog.h>

#include <tempstore/UserRatingProvider.hpp>

#include "UserRatingWatcher.hpp"

namespace rating_calculator {

  namespace service {

    constexpr auto secondsInWeek = 7 * 24 * 3600;

    UserRatingWatcher::UserRatingWatcher(const std::chrono::seconds& ratingUpdateTimeout, std::size_t nRatingPositions,
                                         const core::IDataStoreFactory::Ptr& dataStoreFactory,
                                         const core::ITransportServer::Ptr& transport)
        : ratingUpdateTimeout_(ratingUpdateTimeout), nRatingPositions_(nRatingPositions), transport_(transport),
          userRatingProvider_(core::TimeHelper::WeekDay::Monday, std::chrono::seconds(secondsInWeek), dataStoreFactory)
    {}

    void UserRatingWatcher::start()
    {
      ratingUpdateThread_ = std::jthread(
          [this](std::stop_token st)
          {
            while (true)
            {
              std::vector<core::UserIdentifier> snapshot;
              {
                std::unique_lock<std::mutex> lck(mutex_);
                if (!cv_.wait(lck, st,
                              [this]()
                              {
                                return !connectedUsers_.empty();
                              }))
                {
                  break;
                }
                snapshot.assign(connectedUsers_.begin(), connectedUsers_.end());
              }

              for (core::UserIdentifier userId : snapshot)
              {
                if (st.stop_requested())
                {
                  break;
                }
                sendUserRelativeRating(userId);
              }

              {
                std::binary_semaphore sem{0};
                std::stop_callback cb(st,
                                      [&sem]
                                      {
                                        sem.release();
                                      });
                sem.try_acquire_for(ratingUpdateTimeout_);
              }
              if (st.stop_requested())
              {
                break;
              }
            }
          });
    }

    void UserRatingWatcher::stop()
    {
      ratingUpdateThread_.request_stop();
      if (ratingUpdateThread_.joinable())
      {
        ratingUpdateThread_.join();
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
