/*
 * ApplicationService.hpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_APPLICATIONSERVICE_HPP
#define RATINGCALCULATOR_APPLICATIONSERVICE_HPP

#include <chrono>
#include <memory>

#include <boost/noncopyable.hpp>

#include <core/ITransportServer.hpp>
#include <core/IDataStoreFactory.hpp>

#include "UserRatingWatcher.hpp"

namespace rating_calculator {

  namespace service {

    /**
     * @brief Wires transport, storage, and rating broadcast together.
     *
     * Transport-agnostic: accepts any ITransportServer implementation.
     * The concrete transport (WS, gRPC, Kafka, RabbitMQ) is selected and
     * constructed by Application before being passed here.
     */
    class ApplicationService : public std::enable_shared_from_this<ApplicationService>, boost::noncopyable {
    public:
      using Ptr = std::shared_ptr<ApplicationService>;

      /**
       * @param transport       Ready-to-use transport (not yet started).
       * @param timeout         Rating broadcast interval.
       * @param nRatingPositions Neighbour positions to include per user.
       */
      ApplicationService(const core::ITransportServer::Ptr& transport, const std::chrono::seconds& timeout,
                         size_t nRatingPositions = 10);

      int run();

    private:
      void dispatchMessage(core::BaseMessage::Ptr message);

    private:
      core::ITransportServer::Ptr transport_;
      core::IDataStoreFactory::Ptr dataStoreFactory_;
      UserRatingWatcher userRatingWatcher_;
    };

  } // namespace service

} // namespace rating_calculator

#endif // RATINGCALCULATOR_APPLICATIONSERVICE_HPP
