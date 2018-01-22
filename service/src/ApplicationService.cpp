/*
 * ApplicationService.cpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <memory>
#include <thread>
#include <string>

#include <core/Types.hpp>
#include <core/BaseException.hpp>
#include <core/ulog.h>

#include <tempstore/DataStoreFactory.hpp>

#include <webapi/transport/WsProtocol.hpp>

#include "ApplicationService.hpp"

namespace rating_calculator {

  namespace service {

    ApplicationService::ApplicationService(int port, int timeout, size_t threadPoolSize):
    protocol(std::make_shared<webapi::transport::WsProtocol<WsServer>>()), dataStoreFactory(std::make_shared<tempstore::DataStoreFactory>()), userRatingWatcher(timeout, 10, dataStoreFactory, protocol)
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
        try
        {
          core::BaseMessage::Ptr baseMessage = selfType->protocol->parseMessage(message, connection);

          if (!baseMessage)
          {
            // The protocol service message was received.
            return;
          }

          auto& userDataStore = selfType->dataStoreFactory->getUserDataStore();
          auto& userDealDataStore = selfType->dataStoreFactory->getUserDealDataStore();

          if (baseMessage->getType() == core::MessageType::UserRegistered)
          {
            auto userRegisteredMessage = std::static_pointer_cast<core::Message<core::UserInformation>>(baseMessage);
            userDataStore.addUser(userRegisteredMessage->getData());
          }
          else if (baseMessage->getType() == core::MessageType::UserRenamed)
          {
            auto userRenamedMessage = std::static_pointer_cast<core::Message<core::UserInformation>>(baseMessage);
            userDataStore.renameUser(userRenamedMessage->getData().id, userRenamedMessage->getData().name);
          }
          else if (baseMessage->getType() == core::MessageType::UserConnected)
          {
            auto userConnectedMessage = std::static_pointer_cast<core::Message<core::UserIdInformation>>(baseMessage);
            selfType->userRatingWatcher.userConnected(userConnectedMessage->getData().id, connection);
          }
          else if (baseMessage->getType() == core::MessageType::UserDisconnected)
          {
            auto userDisconnectedMessage = std::static_pointer_cast<core::Message<core::UserIdInformation>>(
                    baseMessage);
            selfType->userRatingWatcher.userDisconnected(userDisconnectedMessage->getData().id);
          }
          else if (baseMessage->getType() == core::MessageType::UserDealWon)
          {
            auto userDealWonMessage = std::static_pointer_cast<core::Message<core::DealInformation>>(baseMessage);
            userDealDataStore.addDeal(userDealWonMessage->getData());
          }
          else
          {
            const auto& enumConverter = core::EnumConverter<core::MessageType>::get_const_instance();
            mdebug_warn("Skip unsupported message type '%s' processing.",
                        enumConverter.toString(baseMessage->getType()).c_str());
          }
        }
        catch(const core::BaseException& exc)
        {
          mdebug_error("%s", exc.what());
        }
        catch(const std::exception& exc)
        {
          mdebug_error("Unknown error occurred: %s", exc.what());
        }

      };

      echo.on_open = [](std::shared_ptr<WsServer::Connection> connection) {
        mdebug_info("Client connected: %s:%d (0x%x)", connection->remote_endpoint_address().c_str(), connection->remote_endpoint_port(), connection.get());
      };

      // See RFC 6455 7.4.1. for status codes
      echo.on_close = [](std::shared_ptr<WsServer::Connection> connection, int status, const std::string & /*reason*/) {
        mdebug_info("Connection %s:%d (0x%x) closed with status code: %d.", connection->remote_endpoint_address().c_str(), connection->remote_endpoint_port(), connection.get(), status);
      };

     echo.on_error = [selfType](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
        mdebug_error("Error in connection 0x%x. Error: %s (%d).", connection.get(), ec.message().c_str(), ec);
        selfType->server.stop();
      };
    }

    int ApplicationService::run()
    {
      setWsEndpoints();

      protocol->start();
      userRatingWatcher.start();

      auto selfCopy = shared_from_this();

      std::thread serverThread([selfCopy]() {
        selfCopy->server.start();
      });

      serverThread.join();
      userRatingWatcher.stop();
      protocol->stop();

      return 0;
    }

  }

}
