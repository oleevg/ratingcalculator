/*
 * TestUser.cpp
 *
 *  Created on: 1/8/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include "TestUser.hpp"

namespace rating_calculator {

  namespace test_client {

    TestUser::TestUser(const core::UserIdentifier& id, const std::string& name):
      userInformation(id, name), connected(false)
    {

    }

    void TestUser::changeName(const std::string& name)
    {
      userInformation.name = name;
    }

    bool TestUser::isConnected() const
    {
      return connected;
    }

    void TestUser::setConnected(bool connected)
    {
      this->connected = connected;
    }

    const std::string& TestUser::getName() const
    {
      return userInformation.name;
    }
  }
  }
