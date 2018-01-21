/*
 * DataStore.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_DATASTORE_HPP
#define RATINGCALCULATOR_DATASTORE_HPP

#include <core/IDataStoreFactory.hpp>

namespace rating_calculator {

  namespace tempstore {

    class DataStoreFactory : public core::IDataStoreFactory {
      public:
        core::IUserDataStore& getUserDataStore() override;

        core::IUserDealDataStore& getUserDealDataStore() override;
    };

  }

}


#endif //RATINGCALCULATOR_DATASTORE_HPP
