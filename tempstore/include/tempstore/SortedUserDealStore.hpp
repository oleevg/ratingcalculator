/*
 * SortedUserDealStore.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_SORTEDUSERDEALSTORE_HPP
#define RATINGCALCULATOR_SORTEDUSERDEALSTORE_HPP

#include <map>
#include <cstdint>
#include <chrono>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>

#include <core/IDataStoreFactory.hpp>
#include <core/Types.hpp>
#include <core/TimeHelper.hpp>

namespace rating_calculator {

  namespace tempstore {

    class SortedUserDealStore {
      public:
        SortedUserDealStore(const core::IDataStoreFactory::Ptr& dataStoreFactory, core::TimeHelper::WeekDay startPeriodDay,
                                    uint64_t periodDuration);
        ~SortedUserDealStore();

        core::UserPosition getUserPosition(const core::UserIdentifier& userIdentifier) const;

        core::UserPositionsCollection getHeadPositions(size_t nPositions) const;
        core::UserPositionsCollection
        getHighPositions(const core::UserIdentifier& userIdentifier, size_t nPositions) const;
        core::UserPositionsCollection
        getLowPositions(const core::UserIdentifier& userIdentifier, size_t nPositions) const;

        void stop();

      private:
        void updatePeriodTime(const core::TimePoint& startTime);

      private:
        core::TimePoint startTime;
        core::TimePoint endTime;
        int64_t periodDuration_;

        std::thread watcherThread;
        std::mutex storeMutex;
        std::atomic<bool> stopped;

        core::IDataStoreFactory::Ptr dataStoreFactory_;
    };

  }

}


#endif //RATINGCALCULATOR_SORTEDUSERDEALSTORE_HPP
