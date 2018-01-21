/*
 * IDataStoreFactory.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_IDATASTOREFACTORY_HPP
#define RATINGCALCULATOR_IDATASTOREFACTORY_HPP

#include <memory>

#include "IUserDataStore.hpp"
#include "IUserDealDataStore.hpp"

namespace rating_calculator {

  namespace core {

    /**
     * @brief Factory class providing access to core interfaces implementations.
     */
    class IDataStoreFactory {
      public:
        typedef std::shared_ptr<IDataStoreFactory> Ptr;

      public:
        virtual ~IDataStoreFactory() = default;

        /**
         * @brief Get user data store implementation.
         * @return User data store instance.
         */
        virtual IUserDataStore& getUserDataStore() = 0;

        /**
         * @brief Get users deal data store implementation.
         * @return Users deal data store instance.
         */
        virtual IUserDealDataStore& getUserDealDataStore() = 0;
    };

  }

}

#endif //RATINGCALCULATOR_IDATASTOREFACTORY_HPP
