/*
 * ApplicationService.hpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_APPLICATIONSERVICE_HPP
#define RATINGCALCULATOR_APPLICATIONSERVICE_HPP

#include <memory>

#include <boost/enable_shared_from_this.hpp>

#include <webapi/websockets/server_ws.hpp>
#include <webapi/transport/WsProtocol.hpp>

#include "UserRatingWatcher.hpp"

namespace rating_calculator {

  namespace service {

    /**
     * @brief The service class describing the application life cycle.
     */
    class ApplicationService : public std::enable_shared_from_this<ApplicationService>, boost::noncopyable{
        typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

      public:
        typedef std::shared_ptr<ApplicationService> Ptr;

      public:
        /**
         * @brief ctor
         * @param port The server's port number to listen.
         * @param timeout User rating update timeout in seconds.
         * @param threadPoolSize The service's thread pool size.
         */
        ApplicationService(int port, int timeout, size_t threadPoolSize);

        /**
         * @brief Starts the application life cycle.
         * @return The application's return code.
         */
        int run();

      private:
        void setWsEndpoints();

      private:
        WsServer server;
        webapi::transport::WsProtocol<WsServer>::Ptr protocol;

        core::IDataStoreFactory::Ptr dataStoreFactory;
        UserRatingWatcher userRatingWatcher;
    };

  }

}


#endif //RATINGCALCULATOR_APPLICATIONSERVICE_HPP
