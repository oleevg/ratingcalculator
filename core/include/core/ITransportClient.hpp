/*
 * ITransportClient.hpp
 *
 *  Created on: 7/4/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include "Model.hpp"

namespace rating_calculator {

  namespace core {

    /**
     * @brief Transport-agnostic client interface.
     *
     * Mirrors ITransportServer on the client side. Implementations
     * (WebSocket, gRPC, …) handle connection lifecycle and serialization
     * internally; the caller only sees BaseMessage objects.
     */
    class ITransportClient {
    public:
      using Ptr = std::shared_ptr<ITransportClient>;

      /**
       * @brief Callback invoked for every application-level message received
       *        from the server. Called from a background thread — must be thread-safe.
       */
      using MessageHandler = std::function<void(BaseMessage::Ptr)>;

      virtual ~ITransportClient() = default;

      virtual void setMessageHandler(MessageHandler handler) = 0;

      /**
       * @brief Start the connection in the background (non-blocking).
       *        Call waitForConnection() afterwards to ensure the link is up
       *        before sending messages.
       */
      virtual void connect() = 0;

      virtual void disconnect() = 0;

      virtual void sendMessage(const BaseMessage::Ptr& message) = 0;

      /**
       * @brief Block until the connection is established or the timeout expires.
       * @return true if connected, false on timeout.
       */
      virtual bool waitForConnection(std::chrono::seconds timeout) = 0;
    };

  } // namespace core

} // namespace rating_calculator