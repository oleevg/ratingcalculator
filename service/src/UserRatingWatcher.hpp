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

#include <tempstore/SortedUserDealStore.hpp>

#include <webapi/transport/WsProtocol.hpp>
#include <webapi/websockets/server_ws.hpp>

namespace rating_calculator {

  namespace service {

    typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;
    typedef WsServer::Connection WsConnection;

    class UserRatingWatcher : public boost::noncopyable {
      private:
        struct UserConnection {
            typedef std::shared_ptr<UserConnection> Ptr;

            UserConnection(const core::UserIdentifier& _userIdentifier,
                           const std::shared_ptr<WsConnection>& _connection);

            core::UserIdentifier userIdentifier;
            std::weak_ptr<WsConnection> connection;
            core::TimePoint connectionTime;
            std::atomic<bool> connected;
        };

        typedef std::unordered_map<core::UserIdentifier, UserConnection::Ptr> UserConnectionCollection;

      public:
        UserRatingWatcher(int ratingUpdateTimeout, size_t nRatingPositions,
                                  const core::IDataStoreFactory::Ptr& dataStoreFactory,
                                  const webapi::transport::WsProtocol<WsServer>::Ptr& protocol);

        void userConnected(const core::UserIdentifier& userIdentifier,
                           const std::shared_ptr<WsConnection>& connection);

        void userDisconnected(const core::UserIdentifier& userIdentifier);

        void start();

        void stop();

      private:
        void sendUserRelativeRating(const core::UserIdentifier& userIdentifier,
                                    const std::weak_ptr<WsConnection>& connection) const;

      private:
        size_t nRatingPositions_;
        int ratingUpdateTimeout_;
        webapi::transport::WsProtocol<WsServer>::Ptr protocol_;

        std::thread ratingUpdateThread_;
        std::mutex userConnectionsMutex_;
        std::condition_variable userConnectionsCondVar_;
        UserConnectionCollection userConnections_;

        tempstore::SortedUserDealStore sortedDealStore_;
        core::IDataStoreFactory::Ptr dataStoreFactory_;

        std::atomic<bool> stopped_;
    };


  }

}


#endif //RATINGCALCULATOR_USERRATINGWATCHER_HPP
