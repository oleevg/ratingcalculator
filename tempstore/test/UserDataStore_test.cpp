/*
 * UserDataStore_test.cpp
 *
 *  Created on: 11/9/19
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#define BOOST_TEST_MODULE "User data store test module"

#include "../src/UserDataStore.hpp"

#include <boost/test/unit_test.hpp>
#include <core/BaseException.hpp>

BOOST_AUTO_TEST_SUITE(UserDataStore)

  namespace tempstore = rating_calculator::tempstore;
  namespace core = rating_calculator::core;

  BOOST_AUTO_TEST_CASE(Must_throw_when_asking_for_nonpresent_user)
  {
    tempstore::UserDataStore userDataStore;

    BOOST_REQUIRE_THROW(userDataStore.getUserInformation(123), core::BaseException);
  }

  BOOST_AUTO_TEST_CASE(Should_return_added_user)
  {
    tempstore::UserDataStore userDataStore;

    core::UserInformation userInformation{123, "User123"};
    userDataStore.addUser(userInformation);

    const auto& returnedUserInformation = userDataStore.getUserInformation(userInformation.id);

    BOOST_REQUIRE(userInformation == returnedUserInformation);
  }

  BOOST_AUTO_TEST_CASE(Must_throw_when_renaming_for_nonpresent_user)
  {
    tempstore::UserDataStore userDataStore;

    BOOST_REQUIRE_THROW(userDataStore.renameUser(123, "NewUser123"), core::BaseException);
  }

  BOOST_AUTO_TEST_CASE(Should_rename_user)
  {
    tempstore::UserDataStore userDataStore;

    core::UserInformation userInformation{123, "User123"};
    userDataStore.addUser(userInformation);
    userDataStore.renameUser(userInformation.id, "NewUser123");

    const auto& returnedUserInformation = userDataStore.getUserInformation(userInformation.id);

    BOOST_REQUIRE_EQUAL(userInformation.id, returnedUserInformation.id);
    BOOST_REQUIRE_EQUAL("NewUser123", returnedUserInformation.name);
  }

BOOST_AUTO_TEST_SUITE_END()

