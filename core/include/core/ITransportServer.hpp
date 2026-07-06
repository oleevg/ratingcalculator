/*
 * ITransportServer.hpp
 *
 *  Created on: 7/3/26
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#pragma once

#include <functional>
#include <memory>

#include "Model.hpp"
#include "Types.hpp"

namespace rating_calculator {

  namespace core {

    /**
     * @brief Transport-agnostic server interface.
     *
     * Implementations handle serialization, connection lifecycle, and delivery
     * guarantees internally.  The application layer only sees parsed BaseMessage
     * objects coming in and calls sendToUser() to push messages out.
     */
    class ITransportServer {
    public:
      using Ptr = std::shared_ptr<ITransportServer>;

      /**
       * @brief Callback invoked for every application-level message received from any client.
       *        Called from the transport's I/O thread — implementations must be thread-safe.
       */
      using MessageHandler = std::function<void(BaseMessage::Ptr)>;

      virtual ~ITransportServer() = default;

      /**
       * @brief Register the callback that receives incoming application messages.
       *        Must be called before start().
       */
      virtual void setMessageHandler(MessageHandler handler) = 0;

      /**
       * @brief Start background I/O machinery (non-blocking).
       */
      virtual void start() = 0;

      /**
       * @brief Block until stop() is called.  Intended to be the main-thread's blocking call.
       */
      virtual void run() = 0;

      /**
       * @brief Signal the transport to shut down and wait for all threads to join.
       */
      virtual void stop() = 0;

      /**
       * @brief Push a message to a specific connected user.
       *        For queue-based transports (Kafka, RabbitMQ) this publishes to a
       *        per-user topic / routing key instead of a live session.
       */
      virtual void sendToUser(UserIdentifier userId, const BaseMessage::Ptr& message) = 0;
    };

  } // namespace core

} // namespace rating_calculator
