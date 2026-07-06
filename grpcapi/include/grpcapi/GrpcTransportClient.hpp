/*
 * GrpcTransportClient.hpp
 *
 *  Created on: 7/4/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#pragma once

#include <memory>

#include <core/ITransportClient.hpp>

namespace rating_calculator {

  namespace grpcapi {

    /**
     * @brief ITransportClient backed by a gRPC bidirectional streaming RPC.
     *
     * Connects to a GrpcTransportServer and exchanges the same application
     * messages (UserRegistered, UserDealWon, UserRelativeRating, …) as the
     * WebSocket client — just over gRPC/HTTP2 instead.
     *
     * gRPC internals are hidden behind a PIMPL so callers do not need to
     * include any gRPC or protobuf headers.
     */
    class GrpcTransportClient : public core::ITransportClient {
    public:
      using Ptr = std::shared_ptr<GrpcTransportClient>;

      GrpcTransportClient(const std::string& host, int port);
      ~GrpcTransportClient();

      void setMessageHandler(MessageHandler handler) override;
      void connect() override;
      void disconnect() override;
      void sendMessage(const core::BaseMessage::Ptr& message) override;
      bool waitForConnection(std::chrono::seconds timeout) override;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };

  } // namespace grpcapi

} // namespace rating_calculator