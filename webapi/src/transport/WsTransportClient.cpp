/*
 * WsTransportClient.cpp
 *
 *  Created on: 7/4/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <core/ulog.h>

#include <webapi/transport/WsTransportClient.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      WsTransportClient::WsTransportClient(const std::string& host, int port)
          : client_(host + ":" + std::to_string(port) + "/rating")
      {}

      WsTransportClient::~WsTransportClient()
      {
        disconnect();
      }

      void WsTransportClient::setMessageHandler(MessageHandler handler)
      {
        messageHandler_ = std::move(handler);
      }

      void WsTransportClient::connect()
      {
        protocol_.start();

        client_.on_message =
            [this](std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::Message> rawMessage)
        {
          auto message = protocol_.parseMessage(rawMessage, connection);
          if (message && messageHandler_)
          {
            messageHandler_(std::move(message));
          }
        };

        client_.on_open = [this](std::shared_ptr<WsClient::Connection> connection)
        {
          std::lock_guard<std::mutex> lck(connMutex_);
          connection_ = connection;
          connected_.store(true);
          connCondVar_.notify_all();
          mdebug_info("WsTransportClient: connected.");
        };

        client_.on_close = [this](std::shared_ptr<WsClient::Connection>, int status, const std::string&)
        {
          mdebug_info("WsTransportClient: connection closed (status %d).", status);
          stopped_.store(true);
          connCondVar_.notify_all();
        };

        client_.on_error = [this](std::shared_ptr<WsClient::Connection>, const SimpleWeb::error_code& ec)
        {
          mdebug_error("WsTransportClient: error: %s (%d).", ec.message().c_str(), ec.value());
          stopped_.store(true);
          connCondVar_.notify_all();
        };

        clientThread_ = std::thread(
            [this]()
            {
              client_.start();
            });
      }

      void WsTransportClient::disconnect()
      {
        stopped_.store(true);
        connCondVar_.notify_all();
        client_.stop();
        if (clientThread_.joinable())
        {
          clientThread_.join();
        }
      }

      void WsTransportClient::sendMessage(const core::BaseMessage::Ptr& message)
      {
        std::shared_ptr<WsClient::Connection> conn;
        {
          std::lock_guard<std::mutex> lck(connMutex_);
          conn = connection_;
        }
        if (conn)
        {
          protocol_.sendMessage(message, conn);
        }
      }

      bool WsTransportClient::waitForConnection(std::chrono::seconds timeout)
      {
        std::unique_lock<std::mutex> lck(connMutex_);
        connCondVar_.wait_for(lck, timeout,
                              [this]()
                              {
                                return connected_.load() || stopped_.load();
                              });
        return connected_.load();
      }

    } // namespace transport

  } // namespace webapi

} // namespace rating_calculator
