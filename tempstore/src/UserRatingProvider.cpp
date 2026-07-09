/*
 * UserRatingProvider.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <algorithm>
#include <chrono>
#include <ctime>
#include <ratio>
#include <semaphore>
#include <stop_token>

#include <core/ulog.h>

#include <tempstore/UserRatingProvider.hpp>

namespace rating_calculator {

  namespace tempstore {

    UserRatingProvider::UserRatingProvider(core::TimeHelper::WeekDay startPeriodDay,
                                           std::chrono::seconds periodDuration,
                                           const core::IDataStoreFactory::Ptr& dataStoreFactory)
        : periodDuration_(periodDuration), dataStoreFactory_(dataStoreFactory), sortedDealContainer_(UINT16_MAX)
    {
      updatePeriod(core::TimeHelper::getPreviousWeekDay(std::chrono::system_clock::now(), startPeriodDay));

      auto& userDealDataStore = dataStoreFactory_->getUserDealDataStore();
      userDealDataStore.addDealAddedSlot(
          [this](const core::DealInformation& dealInformation)
          {
            addDeal(dealInformation);
          });

      watcherThread_ = std::jthread(
          [this](std::stop_token st)
          {
            while (!st.stop_requested())
            {
              {
                std::binary_semaphore sem{0};
                std::stop_callback cb(st,
                                      [&sem]
                                      {
                                        sem.release();
                                      });
                sem.try_acquire_until(endTime_);
                if (st.stop_requested())
                {
                  break;
                }
              }

              {
                std::lock_guard<std::mutex> storeLock(storeMutex_);
                sortedDealContainer_.clear();
              }
              mdebug_info("Cleared sorted deal table.");
              updatePeriod(endTime_);
            }
          });
    }

    UserRatingProvider::~UserRatingProvider() = default;

    bool UserRatingProvider::isUserPresent(const core::UserIdentifier& userIdentifier) const
    {
      return sortedDealContainer_.isPresent(userIdentifier);
    }

    core::UserPositionsCollection UserRatingProvider::getHeadPositions(std::size_t nPositions) const
    {
      core::UserPositionsCollection result;
      result.reserve(nPositions);

      auto& userDataStore = dataStoreFactory_->getUserDataStore();

      std::lock_guard<std::mutex> lck(storeMutex_);
      auto userRatingCollection = sortedDealContainer_.getHeadPositions(nPositions);

      std::transform(userRatingCollection.cbegin(), userRatingCollection.cend(), std::back_inserter(result),
                     [&userDataStore](const auto& r) {
                       return core::UserPosition(userDataStore.getUserInformation(r.value.id), r.position, r.value.amount);
                     });

      return result;
    }

    core::UserPositionsCollection UserRatingProvider::getHighPositions(const core::UserIdentifier& userIdentifier,
                                                                       std::size_t nPositions) const
    {
      core::UserPositionsCollection result;
      result.reserve(nPositions);

      auto& userDataStore = dataStoreFactory_->getUserDataStore();

      std::lock_guard<std::mutex> lck(storeMutex_);
      auto userRatingCollection = sortedDealContainer_.getHighPositions(userIdentifier, nPositions);

      std::transform(userRatingCollection.cbegin(), userRatingCollection.cend(), std::back_inserter(result),
                     [&userDataStore](const auto& r) {
                       return core::UserPosition(userDataStore.getUserInformation(r.value.id), r.position, r.value.amount);
                     });

      return result;
    }

    core::UserPositionsCollection UserRatingProvider::getLowPositions(const core::UserIdentifier& userIdentifier,
                                                                      std::size_t nPositions) const
    {
      core::UserPositionsCollection result;
      result.reserve(nPositions);

      auto& userDataStore = dataStoreFactory_->getUserDataStore();

      std::lock_guard<std::mutex> lck(storeMutex_);
      auto userRatingCollection = sortedDealContainer_.getLowPositions(userIdentifier, nPositions);

      std::transform(userRatingCollection.cbegin(), userRatingCollection.cend(), std::back_inserter(result),
                     [&userDataStore](const auto& r) {
                       return core::UserPosition(userDataStore.getUserInformation(r.value.id), r.position, r.value.amount);
                     });

      return result;
    }

    core::UserPosition UserRatingProvider::getUserPosition(const core::UserIdentifier& userIdentifier) const
    {
      auto& userDataStore = dataStoreFactory_->getUserDataStore();

      std::lock_guard<std::mutex> lck(storeMutex_);
      auto userRating = sortedDealContainer_.getPosition(userIdentifier);

      return core::UserPosition(userDataStore.getUserInformation(userIdentifier), userRating.position,
                                userRating.value.amount);
    }

    void UserRatingProvider::updatePeriod(const core::TimePoint& startTime)
    {
      startTime_ = startTime;
      endTime_ = startTime + std::chrono::seconds(periodDuration_);
      mdebug_info("Updated rating period: startTime = '%s', endTime = '%s', now = '%s'.",
                  core::TimeHelper::toString(startTime_).c_str(), core::TimeHelper::toString(endTime_).c_str(),
                  core::TimeHelper::toString(std::chrono::system_clock::now()).c_str());
    }

    void UserRatingProvider::addDeal(const core::DealInformation& dealInformation)
    {
      if (dealInformation.timestamp < std::chrono::system_clock::to_time_t(startTime_))
      {
        mdebug_notice("Ignoring deal as its timestamp < startTime");
      }
      else if (dealInformation.timestamp >= std::chrono::system_clock::to_time_t(endTime_))
      {
        mdebug_notice("Ignoring deal as its timestamp > endTime");
      }
      else
      {
        std::lock_guard<std::mutex> lck(storeMutex_);
        sortedDealContainer_.insert(MultiKeyData(dealInformation.userId, dealInformation.amount));
      }
    }

  } // namespace tempstore

} // namespace rating_calculator
