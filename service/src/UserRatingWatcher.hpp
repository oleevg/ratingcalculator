/*
 * UserRatingWatcher.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_USERRATINGWATCHER_HPP
#define RATINGCALCULATOR_USERRATINGWATCHER_HPP

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include <boost/noncopyable.hpp>

#include <core/TimeHelper.hpp>

#include <tempstore/UserRatingProvider.hpp>

#include <webapi/transport/WsProtocol.hpp>
#include <webapi/websockets/server_ws.hpp>

namespace rating_calculator {

  namespace service {

    typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;
    typedef WsServer::Connection WsConnection;

    /**
     * @brief Class responsible for periodical informing connected users about its relative rating.
     */
    class UserRatingWatcher : public boost::noncopyable {
      private:
        struct UserConnection {
            typedef std::shared_ptr<UserConnection> Ptr;

            UserConnection(const core::UserIdentifier& _userIdentifier,
                           const std::shared_ptr<WsConnection>& _connection);

            core::UserIdentifier userIdentifier;
            std::weak_ptr<WsConnection> connection;
            core::TimePoint connectionTime; // not used so far
            std::atomic<bool> connected;
        };

        typedef std::unordered_map<core::UserIdentifier, UserConnection::Ptr> UserConnectionCollection;

      public:
        /**
         * @brief ctor
         * @param ratingUpdateTimeout The rating update timeout in seconds.
         * @param nRatingPositions The number of positions to include in relative rating data.
         * @param dataStoreFactory The instance of factory class responsible for creating implementations of core data store interfaces.
         * @param protocol The instance of WsProtocol class used for transport channel handling.
         */
        UserRatingWatcher(int ratingUpdateTimeout, size_t nRatingPositions,
                                  const core::IDataStoreFactory::Ptr& dataStoreFactory,
                                  const webapi::transport::WsProtocol<WsServer>::Ptr& protocol);

        /**
         * @brief Handles user connection event.
         * @param userIdentifier User's identifier.
         * @param connection User's corresponding connection.
         */
        void userConnected(const core::UserIdentifier& userIdentifier,
                           const std::shared_ptr<WsConnection>& connection);

        /**
         * @brief Handles user disconnection event,
         * @param userIdentifier User's identifier.
         */
        void userDisconnected(const core::UserIdentifier& userIdentifier);

        /**
         * @brief Starts the thread responsible for informing connected users about its relative rating.
         */
        void start();

        /**
         * @brief Stops the rating informing thread.
         */
        void stop();

      private:
        void sendUserRelativeRating(const core::UserIdentifier& userIdentifier,
                                    const std::weak_ptr<WsConnection>& connection);

      private:
        size_t nRatingPositions_;
        int ratingUpdateTimeout_;
        webapi::transport::WsProtocol<WsServer>::Ptr protocol_;

        std::thread ratingUpdateThread_;
        std::mutex userConnectionsMutex_;
        std::condition_variable userConnectionsCondVar_;
        UserConnectionCollection userConnections_;

        tempstore::UserRatingProvider userRatingProvider_;
        core::IDataStoreFactory::Ptr dataStoreFactory_;

        std::atomic<bool> stopped_;
    };


  }

}


#endif //RATINGCALCULATOR_USERRATINGWATCHER_HPP
