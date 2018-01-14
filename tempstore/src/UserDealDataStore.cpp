/*
 * UserDealDataStore.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include "UserDealDataStore.hpp"

namespace rating_calculator {

  namespace tempstore {

    void UserDealDataStore::addDeal(const core::DealInformation& dealInformation)
    {
      // TODO: for now don't use all the deals history
    }

    void UserDealDataStore::addDealAddedSlot(const core::DealAddedSignal::slot_type& slot)
    {
      dealAddedSignal.connect(slot);
    }

  }
}