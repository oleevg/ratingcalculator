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

    class IDataStoreFactory {
      public:
        typedef std::shared_ptr<IDataStoreFactory> Ptr;

      public:
        virtual ~IDataStoreFactory() = default;

        virtual IUserDataStore& createUserDataStore() = 0;
        virtual IUserDealDataStore& createUserDealDataStore() = 0;
    };

  }

}

#endif //RATINGCALCULATOR_IDATASTOREFACTORY_HPP
