/*
 * GrpcTransportServer.hpp
 *
 *  Created on: 7/4/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#pragma once

#include <memory>

#include <core/ITransportServer.hpp>

namespace rating_calculator {

  namespace grpcapi {

    /**
     * @brief ITransportServer backed by a gRPC bidirectional streaming RPC.
     *
     * One gRPC stream corresponds to one client process. Multiple users
     * (user IDs) may multiplex over the same stream, mirroring how the
     * WebSocket transport works in the test client. Each incoming ClientEvent
     * carries a user_id that is used to register the stream handle; subsequent
     * sendToUser() calls for that user_id write ServerEvents back on the same
     * stream.
     *
     * gRPC internals (generated headers, grpc++ types) are hidden behind a
     * PIMPL so that this header can be included from transport-agnostic code
     * without pulling in gRPC/protobuf headers.
     */
    class GrpcTransportServer : public core::ITransportServer {
    public:
      using Ptr = std::shared_ptr<GrpcTransportServer>;

      explicit GrpcTransportServer(int port);
      ~GrpcTransportServer();

      void setMessageHandler(MessageHandler handler) override;
      void start() override;
      void run() override;
      void stop() override;
      void sendToUser(core::UserIdentifier userId, const core::BaseMessage::Ptr& message) override;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };

  } // namespace grpcapi

} // namespace rating_calculator