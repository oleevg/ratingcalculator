/*
 * UserRatingProvider.hpp
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

#include <boost/noncopyable.hpp>

#include <core/IDataStoreFactory.hpp>
#include <core/Types.hpp>
#include <core/TimeHelper.hpp>
#include <core/MultiKeyVolatileContainer.hpp>

namespace rating_calculator {

  namespace tempstore {

    /**
     * @brief Class responsible for storing and providing access to users rating information.
     * @detailed Rating calculated for the current period starting from the specified week day and ending in specified 'periodDuration' seconds.
     */
    class UserRatingProvider : public boost::noncopyable {
        struct MultiKeyData {
          MultiKeyData(const core::UserIdentifier& _id, float _amount):
          id(_id), amount(_amount)
          {

          }

          core::UserIdentifier id;
          float amount;
        };

        typedef core::MultiKeyVolatileContainer<core::UserIdentifier, float, MultiKeyData, &MultiKeyData::id, &MultiKeyData::amount> SortedDealContainer;

      public:
        /**
         * @brief ctor.
         * @param startPeriodDay Start day of rating calculation period.
         * @param periodDuration Rating calculation period duration in seconds.
         * @param dataStoreFactory
         */
        UserRatingProvider(core::TimeHelper::WeekDay startPeriodDay, uint64_t periodDuration,
                            const core::IDataStoreFactory::Ptr& dataStoreFactory);

        ~UserRatingProvider();

        /**
         * @brief Provides user rating information for the specified user.
         * @param userIdentifier User identifier to get rating information for.
         * @return User rating information.
         */
        core::UserPosition getUserPosition(const core::UserIdentifier& userIdentifier) const;

        /**
         * @brief Provides users rating information for the first rating table positions.
         * @param nPositions Tne number of users to be included in the result.
         * @return Users rating information.
         */
        core::UserPositionsCollection getHeadPositions(size_t nPositions) const;

        /**
         * @brief Provides users rating information which are higher in rating than the specified user.
         * @param userIdentifier User identifier to get rating information relative to.
         * @param nPositions ne number of users to be included in the result.
         * @return Users rating information.
         */
        core::UserPositionsCollection
        getHighPositions(const core::UserIdentifier& userIdentifier, size_t nPositions) const;

        /**
         * @brief Provides users rating information which are lower in rating than the specified user.
         * @param userIdentifier User identifier to get rating information relative to.
         * @param nPositions ne number of users to be included in the result.
         * @return Users rating information.
         */
        core::UserPositionsCollection
        getLowPositions(const core::UserIdentifier& userIdentifier, size_t nPositions) const;

        /**
         * @brief Adds deal information to the store.
         * @param dealInformation
         */
        void addDeal(const core::DealInformation& dealInformation);

        /**
         * @brief Test for user's information availability.
         * @param userIdentifier User identifier.
         * @return True if the user's rating information is available and False otherwise..
         */
        bool isUserPresent(const core::UserIdentifier& userIdentifier) const;

        /**
         * @brief Starts rating calculation process.
         */
        void start();

        /**
         * @brief Stops rating calculation process.
         */
        void stop();

      private:
        void updatePeriod(const core::TimePoint& startTime);

      private:
        core::TimePoint startTime_;
        core::TimePoint endTime_;
        uint64_t periodDuration_;

        std::thread watcherThread_;
        mutable std::mutex storeMutex_;
        std::atomic<bool> stopped_;

        core::IDataStoreFactory::Ptr dataStoreFactory_;
        SortedDealContainer sortedDealContainer_;
    };

  }

}


#endif //RATINGCALCULATOR_SORTEDUSERDEALSTORE_HPP
