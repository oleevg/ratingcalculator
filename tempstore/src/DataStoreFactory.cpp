/*
 * DataStore.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <tempstore/DataStoreFactory.hpp>

#include "UserDataStore.hpp"
#include "UserDealDataStore.hpp"

namespace rating_calculator {

  namespace tempstore {

    core::IUserDataStore& rating_calculator::tempstore::DataStoreFactory::createUserDataStore()
    {
      static UserDataStore userDataStore;

      return userDataStore;
    }

    core::IUserDealDataStore& DataStoreFactory::createUserDealDataStore()
    {
      static UserDealDataStore userDealDataStore;

      return userDealDataStore;
    }

  }
}