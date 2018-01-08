/*
 * ApplicationService.cpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <memory>
#include <thread>
#include <string>
#include <iostream>

#include <core/ulog.h>

#include <webapi/transport/WsProtocol.hpp>
#include <core/Types.hpp>

#include "ApplicationService.hpp"

namespace rating_calculator {

  namespace service {


    ApplicationService::ApplicationService(int port, int period, size_t threadPoolSize):
    protocol(3, 3)
    {
      server.config.port = port;
      server.config.thread_pool_size = threadPoolSize;
    }

    void ApplicationService::setWsEndpoints()
    {
      auto &echo = server.endpoint["^/rating/?$"];

      auto selfType = shared_from_this();

      echo.on_message = [selfType](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::Message> message)
      {
        webapi::transport::BaseMessage::Ptr baseMessage = selfType->protocol.parseMessage<WsServer>(message, connection);

        if(baseMessage->getType() == webapi::transport::MessageType::UserRegistered)
        {
          auto userRegisteredMessage = std::static_pointer_cast<webapi::transport::Message<core::UserInformation>>(baseMessage);
        }
        else if(baseMessage->getType() == webapi::transport::MessageType::UserConnected)
        {
          auto userConnectedMessage = std::static_pointer_cast<webapi::transport::Message<core::UserIdInformation>>(baseMessage);
        }

      };

      echo.on_open = [](std::shared_ptr<WsServer::Connection> connection) {
        std::cout << "Server: Opened connection " << connection.get() << std::endl;
        mdebug_info("Client connected: %s:%d (0x%x)", connection->remote_endpoint_address().c_str(), connection->remote_endpoint_port(), connection.get());
      };

      // See RFC 6455 7.4.1. for status codes
      echo.on_close = [](std::shared_ptr<WsServer::Connection> connection, int status, const std::string & /*reason*/) {
        mdebug_info("Connection %s:%d (0x%x) closed with status code: %d.", connection->remote_endpoint_address().c_str(), connection->remote_endpoint_port(), connection.get(), status);
      };

      // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
      echo.on_error = [](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
        mdebug_error("Error in connection 0x%x. Error: %s (%d).", connection.get(), ec.message().c_str(), ec);
      };
    }

    int ApplicationService::run()
    {
      setWsEndpoints();

      auto selfCopy = shared_from_this();

      std::thread server_thread([selfCopy]() {
        selfCopy->server.start();
      });

      server_thread.join();

      return 0;
    }

  }

}
