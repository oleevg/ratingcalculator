/*
 * SortedUserDealStore.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <ctime>
#include <ratio>

#include <core/ulog.h>

#include <tempstore/SortedUserDealStore.hpp>

namespace rating_calculator {

  namespace tempstore {

    SortedUserDealStore::SortedUserDealStore(core::TimeHelper::WeekDay startPeriodDay, uint64_t periodDuration,
                                             const core::IDataStoreFactory::Ptr& dataStoreFactory) :
            periodDuration_(periodDuration), stopped_(false), dataStoreFactory_(dataStoreFactory)
    {
      updatePeriod(core::TimeHelper::getPreviousWeekDay(std::chrono::system_clock::now(), startPeriodDay));

      auto& userDealDataStore = dataStoreFactory_->createUserDealDataStore();
      userDealDataStore.addDealAddedSlot([this](const core::DealInformation& dealInformation) {
        if (dealInformation.timestamp < std::chrono::system_clock::to_time_t(startTime_))
        {
          mdebug_info("timestamp < startTime");
        }
        else if (dealInformation.timestamp > std::chrono::system_clock::to_time_t(endTime_))
        {
          mdebug_info("timestamp > startTime");
        }
        else
        {
          // TODO: add
          mdebug_info("Deal added slot invoked.");
        }
      });
    }

    SortedUserDealStore::~SortedUserDealStore()
    {
      stop();
    }

    core::UserPositionsCollection SortedUserDealStore::getHeadPositions(size_t nPositions) const
    {
      return rating_calculator::core::UserPositionsCollection();
    }

    core::UserPositionsCollection
    SortedUserDealStore::getHighPositions(const core::UserIdentifier& userIdentifier, size_t nPositions) const
    {
      return rating_calculator::core::UserPositionsCollection();
    }

    core::UserPositionsCollection
    SortedUserDealStore::getLowPositions(const core::UserIdentifier& userIdentifier, size_t nPositions) const
    {
      return rating_calculator::core::UserPositionsCollection();
    }

    core::UserPosition SortedUserDealStore::getUserPosition(const core::UserIdentifier& userIdentifier) const
    {
      auto& userDataStore = dataStoreFactory_->createUserDataStore();
      return core::UserPosition(userDataStore.getUserInformation(userIdentifier), 0, 0);
    }

    void SortedUserDealStore::updatePeriod(const core::TimePoint& startTime)
    {
      startTime_ = startTime;
      endTime_ = startTime + std::chrono::seconds(periodDuration_);
      mdebug_info("Updated rating period: startTime = %d, endTime = %d, now = %d.", std::chrono::system_clock::to_time_t(startTime_), std::chrono::system_clock::to_time_t(endTime_), std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    }

    void SortedUserDealStore::start()
    {
      if(!stopped_.load())
      {
        watcherThread_ = std::thread([this]() {
          while (!stopped_.load())
          {
            std::this_thread::sleep_until(endTime_);

            std::lock_guard<std::mutex> lck(storeMutex_);
            // TODO: clear store
            mdebug_info("Cleared sorted deal table.");

            updatePeriod(endTime_);
          }
        });
      }
    }

    void SortedUserDealStore::stop()
    {
      bool expected = false;
      if (stopped_.compare_exchange_strong(expected, true))
      {
        watcherThread_.join();
      }
    }

  }

}
