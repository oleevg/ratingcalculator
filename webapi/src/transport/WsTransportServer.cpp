/*
 * WsTransportServer.cpp
 *
 *  Created on: 7/3/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <core/EnumConverter.hpp>
#include <core/Model.hpp>
#include <core/Types.hpp>
#include <core/ulog.h>

#include <webapi/transport/WsTransportServer.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      WsTransportServer::WsTransportServer(int port, std::size_t threadPoolSize)
          : protocol_(std::make_shared<WsProtocol<WsServer>>())
      {
        server_.config.port = port;
        server_.config.thread_pool_size = threadPoolSize;
      }

      void WsTransportServer::setMessageHandler(MessageHandler handler)
      {
        messageHandler_ = std::move(handler);
      }

      void WsTransportServer::setupEndpoints()
      {
        auto& endpoint = server_.endpoint["^/rating/?$"];
        auto self = shared_from_this();

        endpoint.on_message =
            [self](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::Message> rawMessage)
        {
          try
          {
            core::BaseMessage::Ptr message = self->protocol_->parseMessage(rawMessage, connection);
            if (!message)
            {
              return; // protocol-level ACK — no app payload
            }

            // Maintain userId <-> connection mapping for sendToUser()
            if (message->getType() == core::MessageType::UserConnected)
            {
              auto msg = std::dynamic_pointer_cast<core::Message<core::UserIdInformation>>(message);
              const core::UserIdentifier userId = msg->getData().id;

              std::lock_guard<std::mutex> lck(self->connectionsMutex_);
              self->userToConnection_[userId] = connection;
              self->connectionToUser_[connection.get()] = userId;
            }
            else if (message->getType() == core::MessageType::UserDisconnected)
            {
              auto msg = std::dynamic_pointer_cast<core::Message<core::UserIdInformation>>(message);
              const core::UserIdentifier userId = msg->getData().id;

              std::lock_guard<std::mutex> lck(self->connectionsMutex_);
              self->userToConnection_.erase(userId);
              self->connectionToUser_.erase(connection.get());
            }

            if (self->messageHandler_)
            {
              self->messageHandler_(std::move(message));
            }
          } catch (const std::exception& exc)
          {
            mdebug_error("Error processing WebSocket message: %s", exc.what());
          }
        };

        endpoint.on_open = [](std::shared_ptr<WsServer::Connection> connection)
        {
          mdebug_info("Client connected: %s:%d (0x%x)", connection->remote_endpoint_address().c_str(),
                      connection->remote_endpoint_port(), connection.get());
        };

        endpoint.on_close =
            [self](std::shared_ptr<WsServer::Connection> connection, int status, const std::string& /*reason*/)
        {
          mdebug_info("Connection %s:%d (0x%x) closed with status code: %d.",
                      connection->remote_endpoint_address().c_str(), connection->remote_endpoint_port(),
                      connection.get(), status);

          std::lock_guard<std::mutex> lck(self->connectionsMutex_);
          auto it = self->connectionToUser_.find(connection.get());
          if (it != self->connectionToUser_.end())
          {
            self->userToConnection_.erase(it->second);
            self->connectionToUser_.erase(it);
          }
        };

        endpoint.on_error = [self](std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec)
        {
          mdebug_error("Error in connection 0x%x. Error: %s (%d).", connection.get(), ec.message().c_str(), ec);
          self->server_.stop();
        };
      }

      void WsTransportServer::start()
      {
        setupEndpoints();
        protocol_->start();

        auto self = shared_from_this();
        serverThread_ = std::thread(
            [self]()
            {
              self->server_.start();
            });
      }

      void WsTransportServer::run()
      {
        serverThread_.join();
      }

      void WsTransportServer::stop()
      {
        server_.stop();
        protocol_->stop();
        if (serverThread_.joinable())
        {
          serverThread_.join();
        }
      }

      void WsTransportServer::sendToUser(core::UserIdentifier userId, const core::BaseMessage::Ptr& message)
      {
        std::shared_ptr<WsConnection> connection;
        {
          std::lock_guard<std::mutex> lck(connectionsMutex_);
          auto it = userToConnection_.find(userId);
          if (it == userToConnection_.end())
          {
            mdebug_warn("sendToUser: no connection registered for user '%lu'.", userId);
            return;
          }
          connection = it->second.lock();
        }

        if (!connection)
        {
          mdebug_warn("sendToUser: connection for user '%lu' has expired.", userId);
          return;
        }

        protocol_->sendMessage(message, connection);
      }

    } // namespace transport

  } // namespace webapi

} // namespace rating_calculator
