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

    /**
     * @brief Interface describing common users deal data store functionality.
     */
    class IUserDealDataStore {
      public:
        virtual ~IUserDealDataStore() = default;

        /**
         * @brief Adds new deal.
         * @param dealInformation Deal information to be added to the store.
         */
        virtual void addDeal(const DealInformation& dealInformation) = 0;

        /**
         * @brief Registers handler for 'deal added' event.
         * @param slot Handler to be invoked when new deal added.
         */
        virtual void addDealAddedSlot(const DealAddedSignal::slot_type& slot) = 0;
    };

  }

}

#endif //RATINGCALCULATOR_IUSERDEALDATASTORE_HPP
