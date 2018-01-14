/*
 * IUserDealDataStore.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_IUSERDEALDATASTORE_HPP
#define RATINGCALCULATOR_IUSERDEALDATASTORE_HPP

#include <boost/signals2/signal.hpp>

#include "Types.hpp"

namespace rating_calculator {

  namespace core {

    typedef boost::signals2::signal<void (const DealInformation& dealInformation)> DealAddedSignal;

    class IUserDealDataStore {
      public:
        virtual ~IUserDealDataStore() = default;

        virtual void addDeal(const DealInformation& dealInformation) = 0;

        virtual void addDealAddedSlot(const DealAddedSignal::slot_type& slot) = 0;
    };

  }

}

#endif //RATINGCALCULATOR_IUSERDEALDATASTORE_HPP
