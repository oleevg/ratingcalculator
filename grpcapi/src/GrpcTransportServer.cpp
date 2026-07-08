/*
 * GrpcTransportServer.cpp
 *
 *  Created on: 7/4/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>

#include <grpcpp/grpcpp.h>

#include <core/Model.hpp>
#include <core/Types.hpp>
#include <core/ulog.h>

#include "rating.grpc.pb.h"

#include <grpcapi/GrpcTransportServer.hpp>

namespace rating_calculator {

  namespace grpcapi {

    // -------------------------------------------------------------------------
    // Serialization helpers (rc:: proto ↔ core:: domain types)
    // -------------------------------------------------------------------------

    static core::UserIdentifier extractUserId(const rc::ClientEvent& event)
    {
      switch (event.payload_case())
      {
      case rc::ClientEvent::kUserRegistered:
        return event.user_registered().user_id();
      case rc::ClientEvent::kUserRenamed:
        return event.user_renamed().user_id();
      case rc::ClientEvent::kUserConnected:
        return event.user_connected().user_id();
      case rc::ClientEvent::kUserDisconnected:
        return event.user_disconnected().user_id();
      case rc::ClientEvent::kUserDealWon:
        return event.user_deal_won().user_id();
      default:
        return 0;
      }
    }

    static core::BaseMessage::Ptr parseClientEvent(const rc::ClientEvent& event)
    {
      switch (event.payload_case())
      {
      case rc::ClientEvent::kUserRegistered: {
        const auto& r = event.user_registered();
        return std::make_shared<core::Message<core::UserInformation>>(core::MessageType::UserRegistered,
                                                                      core::UserInformation(r.user_id(), r.name()));
      }
      case rc::ClientEvent::kUserRenamed: {
        const auto& r = event.user_renamed();
        return std::make_shared<core::Message<core::UserInformation>>(core::MessageType::UserRenamed,
                                                                      core::UserInformation(r.user_id(), r.name()));
      }
      case rc::ClientEvent::kUserConnected: {
        const auto& r = event.user_connected();
        return std::make_shared<core::Message<core::UserIdInformation>>(core::MessageType::UserConnected,
                                                                        core::UserIdInformation(r.user_id()));
      }
      case rc::ClientEvent::kUserDisconnected: {
        const auto& r = event.user_disconnected();
        return std::make_shared<core::Message<core::UserIdInformation>>(core::MessageType::UserDisconnected,
                                                                        core::UserIdInformation(r.user_id()));
      }
      case rc::ClientEvent::kUserDealWon: {
        const auto& r = event.user_deal_won();
        return std::make_shared<core::Message<core::DealInformation>>(
            core::MessageType::UserDealWon, core::DealInformation(r.user_id(), r.timestamp(), r.amount()));
      }
      default:
        return nullptr;
      }
    }

    static void fillRpcPosition(rc::RpcUserPosition* out, const core::UserPosition& pos)
    {
      out->set_user_id(pos.userInformation.id);
      out->set_user_name(pos.userInformation.name);
      out->set_position(static_cast<std::uint64_t>(pos.position));
      out->set_amount(pos.amount);
    }

    static rc::ServerEvent toServerEvent(const core::BaseMessage::Ptr& message)
    {
      rc::ServerEvent event;
      if (message->getType() != core::MessageType::UserRelativeRating)
      {
        return event;
      }

      auto msg = std::dynamic_pointer_cast<core::Message<core::UserRelativeRating>>(message);
      if (!msg)
        return event;

      const auto& rating = msg->getData();
      auto* r = event.mutable_user_relative_rating();

      fillRpcPosition(r->mutable_user_position(), rating.userPosition);
      for (const auto& pos : rating.headPositions)
        fillRpcPosition(r->add_head_positions(), pos);
      for (const auto& pos : rating.highPositions)
        fillRpcPosition(r->add_high_positions(), pos);
      for (const auto& pos : rating.lowPositions)
        fillRpcPosition(r->add_low_positions(), pos);

      return event;
    }

    // -------------------------------------------------------------------------
    // Impl — holds the gRPC server and the RPC service implementation
    // -------------------------------------------------------------------------

    struct GrpcTransportServer::Impl : public rc::RatingService::Service {
      struct StreamHandle {
        grpc::ServerReaderWriter<rc::ServerEvent, rc::ClientEvent>* stream;
        std::mutex writeMutex;
      };

      int port;
      MessageHandler messageHandler;

      std::unique_ptr<grpc::Server> server;

      std::mutex streamsMutex;
      std::unordered_map<void*, std::shared_ptr<StreamHandle>> streamHandles;
      std::unordered_map<core::UserIdentifier, std::shared_ptr<StreamHandle>> userToHandle;

      // RatingService::Service override
      grpc::Status Connect(grpc::ServerContext* /*ctx*/,
                           grpc::ServerReaderWriter<rc::ServerEvent, rc::ClientEvent>* stream) override
      {
        std::set<core::UserIdentifier> sessionUsers;

        rc::ClientEvent event;
        while (stream->Read(&event))
        {
          core::UserIdentifier userId = extractUserId(event);
          mdebug_notice("Received 'DATA' message: '%s'.", event.DebugString().c_str());
          {
            std::lock_guard<std::mutex> lock(streamsMutex);
            void* key = static_cast<void*>(stream);
            auto& handle = streamHandles[key];
            if (!handle)
            {
              handle = std::make_shared<StreamHandle>();
              handle->stream = stream;
            }
            userToHandle[userId] = handle;
            sessionUsers.insert(userId);
          }

          auto message = parseClientEvent(event);
          if (message && messageHandler)
          {
            messageHandler(std::move(message));
          }
        }

        // Cleanup mappings for this stream session
        {
          std::lock_guard<std::mutex> lock(streamsMutex);
          for (core::UserIdentifier uid : sessionUsers)
          {
            userToHandle.erase(uid);
          }
          streamHandles.erase(static_cast<void*>(stream));
        }

        return grpc::Status::OK;
      }

      void sendToUser(core::UserIdentifier userId, const core::BaseMessage::Ptr& message)
      {
        std::shared_ptr<StreamHandle> handle;
        {
          std::lock_guard<std::mutex> lock(streamsMutex);
          auto it = userToHandle.find(userId);
          if (it == userToHandle.end())
          {
            mdebug_warn("GrpcTransportServer: no stream for user '%lu'.", userId);
            return;
          }
          handle = it->second;
        }

        rc::ServerEvent ev = toServerEvent(message);
        if (!ev.has_user_relative_rating())
          return;

        mdebug_notice("Going to send 'DATA' message: '%s'.", ev.DebugString().c_str());
        std::lock_guard<std::mutex> wlock(handle->writeMutex);
        if (!handle->stream->Write(ev))
        {
          mdebug_warn("GrpcTransportServer: write failed for user '%lu'.", userId);
        }
      }
    };

    // -------------------------------------------------------------------------
    // GrpcTransportServer public API
    // -------------------------------------------------------------------------

    GrpcTransportServer::GrpcTransportServer(int port) : impl_(std::make_unique<Impl>())
    {
      impl_->port = port;
    }

    GrpcTransportServer::~GrpcTransportServer() = default;

    void GrpcTransportServer::setMessageHandler(MessageHandler handler)
    {
      impl_->messageHandler = std::move(handler);
    }

    void GrpcTransportServer::start()
    {
      std::string address = "0.0.0.0:" + std::to_string(impl_->port);
      grpc::ServerBuilder builder;
      builder.AddListeningPort(address, grpc::InsecureServerCredentials());
      builder.RegisterService(impl_.get());
      impl_->server = builder.BuildAndStart();
      mdebug_info("GrpcTransportServer: listening on %s.", address.c_str());
    }

    void GrpcTransportServer::run()
    {
      if (impl_->server)
      {
        impl_->server->Wait();
      }
    }

    void GrpcTransportServer::stop()
    {
      if (impl_->server)
      {
        impl_->server->Shutdown();
      }
    }

    void GrpcTransportServer::sendToUser(core::UserIdentifier userId, const core::BaseMessage::Ptr& message)
    {
      impl_->sendToUser(userId, message);
    }

  } // namespace grpcapi

} // namespace rating_calculator
