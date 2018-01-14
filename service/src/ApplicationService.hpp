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

namespace rating_calculator {

  namespace service {

    class ApplicationService : public std::enable_shared_from_this<ApplicationService>, boost::noncopyable{
        typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

      public:
        typedef std::shared_ptr<ApplicationService> Ptr;

      public:
        ApplicationService(int port, int period, size_t threadPoolSize);
        int run();

      private:
        void setWsEndpoints();

      private:
        WsServer server;
        webapi::transport::WsProtocol<WsServer>::Ptr protocol;
    };

  }

}


#endif //RATINGCALCULATOR_APPLICATIONSERVICE_HPP
