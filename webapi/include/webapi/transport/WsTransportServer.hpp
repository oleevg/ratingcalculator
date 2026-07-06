/*
 * WsTransportServer.hpp
 *
 *  Created on: 7/3/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_WSTRANSPORTSERVER_HPP
#define RATINGCALCULATOR_WSTRANSPORTSERVER_HPP

#include <memory>
#include <thread>
#include <mutex>
#include <unordered_map>

#include <core/ITransportServer.hpp>

#include <webapi/websockets/server_ws.hpp>
#include <webapi/transport/WsProtocol.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      /**
       * @brief ITransportServer implementation over WebSocket (Simple-WebSocket-Server).
       *
       * Wraps WsProtocol (ACK/resend layer) on top of the raw WebSocket server.
       * Internally maintains a userId → connection map populated from UserConnected /
       * UserDisconnected application messages; sendToUser() uses that map to route
       * outgoing messages to the right live connection.
       */
      class WsTransportServer : public core::ITransportServer,
                                public std::enable_shared_from_this<WsTransportServer> {
        using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
        using WsConnection = WsServer::Connection;

      public:
        using Ptr = std::shared_ptr<WsTransportServer>;

        WsTransportServer(int port, std::size_t threadPoolSize);

        void setMessageHandler(MessageHandler handler) override;
        void start() override;
        void run() override;
        void stop() override;
        void sendToUser(core::UserIdentifier userId, const core::BaseMessage::Ptr& message) override;

      private:
        void setupEndpoints();

        WsServer server_;
        WsProtocol<WsServer>::Ptr protocol_;
        MessageHandler messageHandler_;

        std::thread serverThread_;
        std::mutex connectionsMutex_;
        std::unordered_map<core::UserIdentifier, std::weak_ptr<WsConnection>> userToConnection_;
        std::unordered_map<WsConnection*, core::UserIdentifier> connectionToUser_;
      };

    } // namespace transport

  } // namespace webapi

} // namespace rating_calculator

#endif // RATINGCALCULATOR_WSTRANSPORTSERVER_HPP
