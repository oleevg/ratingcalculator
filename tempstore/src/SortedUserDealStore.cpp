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

    SortedUserDealStore::SortedUserDealStore(const core::IDataStoreFactory::Ptr& dataStoreFactory, core::TimeHelper::WeekDay startPeriodDay,
                                                 uint64_t periodDuration) :
      dataStoreFactory_(dataStoreFactory), periodDuration_(periodDuration), stopped(false)
    {
      updatePeriodTime(core::TimeHelper::getPreviousWeekDay(std::chrono::system_clock::now(), startPeriodDay));

      auto& userDealDataStore = dataStoreFactory_->createUserDealDataStore();
      userDealDataStore.addDealAddedSlot([this](const core::DealInformation& dealInformation)
                                         {
                                           if(dealInformation.timestamp < std::chrono::system_clock::to_time_t(startTime))
                                           {
                                              mdebug_info("timestamp < startTime");
                                           }
                                           else if(dealInformation.timestamp > std::chrono::system_clock::to_time_t(endTime))
                                           {
                                             mdebug_info("timestamp > startTime");
                                           }
                                           else
                                           {
                                             // TODO: add
                                           }
                                         });

      watcherThread = std::thread([this]()
                                  {
                                    while (!stopped.load())
                                    {
                                      std::this_thread::sleep_until(endTime);

                                      std::lock_guard<std::mutex> lck(storeMutex);
                                      // TODO: clear store
                                      mdebug_info("Sorted deal table cleared.");

                                      updatePeriodTime(endTime);
                                      mdebug_info("Updated end ");
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

    void SortedUserDealStore::updatePeriodTime(const core::TimePoint& start)
    {
      startTime = start;
      endTime += std::chrono::seconds(periodDuration_);
    }

    void SortedUserDealStore::stop()
    {
      bool expected = false;
      if(stopped.compare_exchange_strong(expected, true))
      {
        watcherThread.join();
      }
    }

  }

  }
