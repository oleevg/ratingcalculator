/*
 * UserRatingProvider.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <ctime>
#include <ratio>

#include <core/ulog.h>

#include <tempstore/UserRatingProvider.hpp>

namespace rating_calculator {

  namespace tempstore {

    UserRatingProvider::UserRatingProvider(core::TimeHelper::WeekDay startPeriodDay, uint64_t periodDuration,
                                             const core::IDataStoreFactory::Ptr& dataStoreFactory) :
            periodDuration_(periodDuration), stopped_(false), dataStoreFactory_(dataStoreFactory), sortedDealContainer_(UINT16_MAX)
    {
      updatePeriod(core::TimeHelper::getPreviousWeekDay(std::chrono::system_clock::now(), startPeriodDay));

      auto& userDealDataStore = dataStoreFactory_->getUserDealDataStore();
      userDealDataStore.addDealAddedSlot([this](const core::DealInformation& dealInformation) {
        addDeal(dealInformation);
      });
    }

    UserRatingProvider::~UserRatingProvider()
    {
      stop();
    }

    bool UserRatingProvider::isUserPresent(const core::UserIdentifier& userIdentifier) const
    {
      return sortedDealContainer_.isPresent(userIdentifier);
    }

    core::UserPositionsCollection UserRatingProvider::getHeadPositions(size_t nPositions) const
    {
      core::UserPositionsCollection result;
      result.reserve(nPositions);

      auto& userDataStore = dataStoreFactory_->getUserDataStore();
      auto userRatingCollection = sortedDealContainer_.getHeadPositions(nPositions);

      for (const auto& userRating : userRatingCollection)
      {
        result.emplace_back(userDataStore.getUserInformation(userRating.value.id), userRating.position, userRating.value.amount);
      }

      return result;
    }

    core::UserPositionsCollection
    UserRatingProvider::getHighPositions(const core::UserIdentifier& userIdentifier, size_t nPositions) const
    {
      core::UserPositionsCollection result;
      result.reserve(nPositions);

      auto& userDataStore = dataStoreFactory_->getUserDataStore();

      std::lock_guard<std::mutex> lck(storeMutex_);
      auto userRatingCollection = sortedDealContainer_.getHighPositions(userIdentifier, nPositions);

      for (const auto& userRating : userRatingCollection)
      {
        result.emplace_back(userDataStore.getUserInformation(userRating.value.id), userRating.position, userRating.value.amount);
      }

      return result;
    }

    core::UserPositionsCollection
    UserRatingProvider::getLowPositions(const core::UserIdentifier& userIdentifier, size_t nPositions) const
    {
      core::UserPositionsCollection result;
      result.reserve(nPositions);

      auto& userDataStore = dataStoreFactory_->getUserDataStore();

      std::lock_guard<std::mutex> lck(storeMutex_);
      auto userRatingCollection = sortedDealContainer_.getLowPositions(userIdentifier, nPositions);

      for (const auto& userRating : userRatingCollection)
      {
        result.emplace_back(userDataStore.getUserInformation(userRating.value.id), userRating.position, userRating.value.amount);
      }

      return result;
    }

    core::UserPosition UserRatingProvider::getUserPosition(const core::UserIdentifier& userIdentifier) const
    {
      auto& userDataStore = dataStoreFactory_->getUserDataStore();

      std::lock_guard<std::mutex> lck(storeMutex_);

      auto userRating = sortedDealContainer_.getPosition(userIdentifier);

      return core::UserPosition(userDataStore.getUserInformation(userIdentifier), userRating.position, userRating.value.amount);
    }

    void UserRatingProvider::updatePeriod(const core::TimePoint& startTime)
    {
      startTime_ = startTime;
      endTime_ = startTime + std::chrono::seconds(periodDuration_);
      mdebug_info("Updated rating period: startTime = '%s', endTime = '%s', now = '%s'.", core::TimeHelper::toString(startTime_).c_str(), core::TimeHelper::toString(endTime_).c_str(), core::TimeHelper::toString(std::chrono::system_clock::now()).c_str());
    }

    void UserRatingProvider::start()
    {
      if(!stopped_.load())
      {
        watcherThread_ = std::thread([this]() {
          while (!stopped_.load())
          {
            std::this_thread::sleep_until(endTime_);

            std::lock_guard<std::mutex> lck(storeMutex_);
            sortedDealContainer_.clear();
            mdebug_info("Cleared sorted deal table.");

            updatePeriod(endTime_);
          }
        });
      }
    }

    void UserRatingProvider::stop()
    {
      bool expected = false;
      if (stopped_.compare_exchange_strong(expected, true))
      {
        watcherThread_.join();
      }
    }

    void UserRatingProvider::addDeal(const core::DealInformation& dealInformation)
    {
      if (dealInformation.timestamp < std::chrono::system_clock::to_time_t(startTime_))
      {
        mdebug_notice("Ignoring deal as its timestamp < startTime");
      }
      else if (dealInformation.timestamp >= std::chrono::system_clock::to_time_t(endTime_))
      {
        mdebug_notice("Ignoring deal as its timestamp > startTime");
      }
      else
      {
        std::lock_guard<std::mutex> lck(storeMutex_);
        sortedDealContainer_.insert(MultiKeyData(dealInformation.userId, dealInformation.amount));
      }
    }

  }

}
