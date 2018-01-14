/*
 * UserDealDataStore.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_USERDEALDATASTORE_HPP
#define RATINGCALCULATOR_USERDEALDATASTORE_HPP

#include <core/IUserDealDataStore.hpp>

namespace rating_calculator {

  namespace tempstore {

    class UserDealDataStore : public core::IUserDealDataStore {
      public:
        void addDeal(const core::DealInformation& dealInformation) override;

        void addDealAddedSlot(const core::DealAddedSignal::slot_type& slot) override;

      private:
        core::DealAddedSignal dealAddedSignal;
    };

  }
}


#endif //RATINGCALCULATOR_USERDEALDATASTORE_HPP
