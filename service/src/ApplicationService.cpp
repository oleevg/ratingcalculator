/*
 * ApplicationService.cpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <memory>

#include <core/BaseException.hpp>
#include <core/EnumConverter.hpp>
#include <core/Types.hpp>
#include <core/ulog.h>

#include <tempstore/DataStoreFactory.hpp>

#include "ApplicationService.hpp"

namespace rating_calculator {

  namespace service {

    ApplicationService::ApplicationService(const core::ITransportServer::Ptr& transport,
                                           const std::chrono::seconds& timeout, size_t nRatingPositions)
        : transport_(transport), dataStoreFactory_(std::make_shared<tempstore::DataStoreFactory>()),
          userRatingWatcher_(timeout, nRatingPositions, dataStoreFactory_, transport_)
    {}

    void ApplicationService::dispatchMessage(core::BaseMessage::Ptr message)
    {
      if (!message)
      {
        return;
      }

      try
      {
        auto& userDataStore = dataStoreFactory_->getUserDataStore();
        auto& userDealDataStore = dataStoreFactory_->getUserDealDataStore();

        if (message->getType() == core::MessageType::UserRegistered)
        {
          auto msg = std::dynamic_pointer_cast<core::Message<core::UserInformation>>(message);
          userDataStore.addUser(msg->getData());
        }
        else if (message->getType() == core::MessageType::UserRenamed)
        {
          auto msg = std::dynamic_pointer_cast<core::Message<core::UserInformation>>(message);
          userDataStore.renameUser(msg->getData().id, msg->getData().name);
        }
        else if (message->getType() == core::MessageType::UserConnected)
        {
          auto msg = std::dynamic_pointer_cast<core::Message<core::UserIdInformation>>(message);
          userRatingWatcher_.userConnected(msg->getData().id);
        }
        else if (message->getType() == core::MessageType::UserDisconnected)
        {
          auto msg = std::dynamic_pointer_cast<core::Message<core::UserIdInformation>>(message);
          userRatingWatcher_.userDisconnected(msg->getData().id);
        }
        else if (message->getType() == core::MessageType::UserDealWon)
        {
          auto msg = std::dynamic_pointer_cast<core::Message<core::DealInformation>>(message);
          userDealDataStore.addDeal(msg->getData());
        }
        else
        {
          const auto& enumConverter = core::EnumConverter<core::MessageType>::get_const_instance();
          mdebug_warn("Skip unsupported message type '%s'.", enumConverter.toString(message->getType()).c_str());
        }
      } catch (const core::BaseException& exc)
      {
        mdebug_error("%s", exc.what());
      } catch (const std::exception& exc)
      {
        mdebug_error("Unknown error: %s", exc.what());
      }
    }

    int ApplicationService::run()
    {
      auto self = shared_from_this();
      transport_->setMessageHandler(
          [self](core::BaseMessage::Ptr msg)
          {
            self->dispatchMessage(std::move(msg));
          });

      transport_->start();
      userRatingWatcher_.start();

      mdebug_info("Service started.");

      transport_->run(); // blocks until transport stops

      userRatingWatcher_.stop();

      return 0;
    }

  } // namespace service

} // namespace rating_calculator
