/*
 * WsTransportClient.hpp
 *
 *  Created on: 7/4/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_WSTRANSPORTCLIENT_HPP
#define RATINGCALCULATOR_WSTRANSPORTCLIENT_HPP

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include <core/ITransportClient.hpp>

#include <webapi/websockets/client_ws.hpp>
#include <webapi/transport/WsProtocol.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      /**
       * @brief ITransportClient backed by a WebSocket connection.
       *
       * Wraps SimpleWeb::SocketClient + WsProtocol so the caller can
       * use it through the transport-agnostic ITransportClient interface.
       */
      class WsTransportClient : public core::ITransportClient {
      public:
        using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;
        using Ptr = std::shared_ptr<WsTransportClient>;

        WsTransportClient(const std::string& host, int port);
        ~WsTransportClient() override;

        void setMessageHandler(MessageHandler handler) override;
        void connect() override;
        void disconnect() override;
        void sendMessage(const core::BaseMessage::Ptr& message) override;
        bool waitForConnection(std::chrono::seconds timeout) override;

      private:
        MessageHandler messageHandler_;

        WsClient client_;
        WsProtocol<WsClient> protocol_;
        std::shared_ptr<WsClient::Connection> connection_;

        std::thread clientThread_;
        std::mutex connMutex_;
        std::condition_variable connCondVar_;
        std::atomic<bool> connected_{false};
        std::atomic<bool> stopped_{false};
      };

    } // namespace transport

  } // namespace webapi

} // namespace rating_calculator

#endif // RATINGCALCULATOR_WSTRANSPORTCLIENT_HPP
