/*
 * GrpcTransportClient.cpp
 *
 *  Created on: 7/4/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include <grpcpp/grpcpp.h>

#include <core/Model.hpp>
#include <core/Types.hpp>
#include <core/ulog.h>

#include "rating.grpc.pb.h"

#include <grpcapi/GrpcTransportClient.hpp>

namespace rating_calculator {

  namespace grpcapi {

    // -------------------------------------------------------------------------
    // Serialization helpers
    // -------------------------------------------------------------------------

    static rc::ClientEvent toClientEvent(const core::BaseMessage::Ptr& message)
    {
      rc::ClientEvent event;
      switch (message->getType())
      {
      case core::MessageType::UserRegistered: {
        auto msg = std::dynamic_pointer_cast<core::Message<core::UserInformation>>(message);
        auto* r = event.mutable_user_registered();
        r->set_user_id(msg->getData().id);
        r->set_name(msg->getData().name);
        break;
      }
      case core::MessageType::UserRenamed: {
        auto msg = std::dynamic_pointer_cast<core::Message<core::UserInformation>>(message);
        auto* r = event.mutable_user_renamed();
        r->set_user_id(msg->getData().id);
        r->set_name(msg->getData().name);
        break;
      }
      case core::MessageType::UserConnected: {
        auto msg = std::dynamic_pointer_cast<core::Message<core::UserIdInformation>>(message);
        event.mutable_user_connected()->set_user_id(msg->getData().id);
        break;
      }
      case core::MessageType::UserDisconnected: {
        auto msg = std::dynamic_pointer_cast<core::Message<core::UserIdInformation>>(message);
        event.mutable_user_disconnected()->set_user_id(msg->getData().id);
        break;
      }
      case core::MessageType::UserDealWon: {
        auto msg = std::dynamic_pointer_cast<core::Message<core::DealInformation>>(message);
        auto* r = event.mutable_user_deal_won();
        r->set_user_id(msg->getData().userId);
        r->set_timestamp(msg->getData().timestamp);
        r->set_amount(msg->getData().amount);
        break;
      }
      default:
        break;
      }
      return event;
    }

    static core::UserPosition fromRpcPosition(const rc::RpcUserPosition& p)
    {
      return core::UserPosition(core::UserInformation(p.user_id(), p.user_name()),
                                static_cast<std::size_t>(p.position()), p.amount());
    }

    static core::BaseMessage::Ptr fromServerEvent(const rc::ServerEvent& event)
    {
      if (!event.has_user_relative_rating())
        return nullptr;

      const auto& r = event.user_relative_rating();

      core::UserPosition userPos = fromRpcPosition(r.user_position());

      core::UserPositionsCollection headPos, highPos, lowPos;
      for (const auto& p : r.head_positions())
        headPos.push_back(fromRpcPosition(p));
      for (const auto& p : r.high_positions())
        highPos.push_back(fromRpcPosition(p));
      for (const auto& p : r.low_positions())
        lowPos.push_back(fromRpcPosition(p));

      return std::make_shared<core::Message<core::UserRelativeRating>>(
          core::MessageType::UserRelativeRating, core::UserRelativeRating(userPos, headPos, highPos, lowPos));
    }

    // -------------------------------------------------------------------------
    // Impl
    // -------------------------------------------------------------------------

    struct GrpcTransportClient::Impl {
      std::string address;
      MessageHandler messageHandler;

      std::shared_ptr<grpc::Channel> channel;
      std::unique_ptr<rc::RatingService::Stub> stub;
      grpc::ClientContext context;
      std::unique_ptr<grpc::ClientReaderWriter<rc::ClientEvent, rc::ServerEvent>> stream;

      std::thread receiveThread;
      std::mutex writeMutex;
      std::atomic<bool> stopped{false};
    };

    // -------------------------------------------------------------------------
    // GrpcTransportClient public API
    // -------------------------------------------------------------------------

    GrpcTransportClient::GrpcTransportClient(const std::string& host, int port) : impl_(std::make_unique<Impl>())
    {
      impl_->address = host + ":" + std::to_string(port);
    }

    GrpcTransportClient::~GrpcTransportClient()
    {
      disconnect();
    }

    void GrpcTransportClient::setMessageHandler(MessageHandler handler)
    {
      impl_->messageHandler = std::move(handler);
    }

    void GrpcTransportClient::connect()
    {
      impl_->channel = grpc::CreateChannel(impl_->address, grpc::InsecureChannelCredentials());
      impl_->stub = rc::RatingService::NewStub(impl_->channel);
      impl_->stream = impl_->stub->Connect(&impl_->context);

      impl_->receiveThread = std::thread(
          [this]()
          {
            rc::ServerEvent event;
            while (!impl_->stopped.load() && impl_->stream->Read(&event))
            {
              auto message = fromServerEvent(event);
              if (message && impl_->messageHandler)
              {
                impl_->messageHandler(std::move(message));
              }
            }
          });
    }

    void GrpcTransportClient::disconnect()
    {
      if (!impl_->stopped.exchange(true))
      {
        impl_->context.TryCancel();
        if (impl_->receiveThread.joinable())
        {
          impl_->receiveThread.join();
        }
      }
    }

    void GrpcTransportClient::sendMessage(const core::BaseMessage::Ptr& message)
    {
      rc::ClientEvent event = toClientEvent(message);
      if (event.payload_case() == rc::ClientEvent::PAYLOAD_NOT_SET)
        return;

      std::lock_guard<std::mutex> lck(impl_->writeMutex);
      if (!impl_->stream->Write(event))
      {
        mdebug_warn("GrpcTransportClient: write failed.");
      }
    }

    bool GrpcTransportClient::waitForConnection(std::chrono::seconds timeout)
    {
      auto deadline = std::chrono::system_clock::now() + timeout;
      return impl_->channel->WaitForConnected(deadline);
    }

  } // namespace grpcapi

} // namespace rating_calculator
