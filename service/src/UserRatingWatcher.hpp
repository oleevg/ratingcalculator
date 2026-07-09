/*
 * UserRatingWatcher.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_USERRATINGWATCHER_HPP
#define RATINGCALCULATOR_USERRATINGWATCHER_HPP

#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_set>

#include <boost/noncopyable.hpp>

#include <core/ITransportServer.hpp>
#include <core/TimeHelper.hpp>

#include <tempstore/UserRatingProvider.hpp>

namespace rating_calculator {

  namespace service {

    /**
     * @brief Periodically pushes relative rating updates to all connected users.
     */
    class UserRatingWatcher : public boost::noncopyable {
    public:
      /**
       * @param ratingUpdateTimeout  Interval between broadcast cycles.
       * @param nRatingPositions     Neighbour positions to include in each update.
       * @param dataStoreFactory     Storage factory.
       * @param transport            Transport used to send rating messages to users.
       */
      UserRatingWatcher(const std::chrono::seconds& ratingUpdateTimeout, size_t nRatingPositions,
                        const core::IDataStoreFactory::Ptr& dataStoreFactory,
                        const core::ITransportServer::Ptr& transport);

      void userConnected(core::UserIdentifier userId);
      void userDisconnected(core::UserIdentifier userId);

      void start();
      void stop();

    private:
      void sendUserRelativeRating(core::UserIdentifier userId);

    private:
      std::size_t nRatingPositions_;
      std::chrono::seconds ratingUpdateTimeout_;
      core::ITransportServer::Ptr transport_;

      std::mutex mutex_;
      std::condition_variable_any cv_;
      std::unordered_set<core::UserIdentifier> connectedUsers_;

      tempstore::UserRatingProvider userRatingProvider_;

      std::jthread ratingUpdateThread_;
    };

  } // namespace service

} // namespace rating_calculator

#endif // RATINGCALCULATOR_USERRATINGWATCHER_HPP
