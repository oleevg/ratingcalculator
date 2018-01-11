/*
 * UserSimulator.cpp
 *
 *  Created on: 1/8/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include "UserSimulator.hpp"

namespace rating_calculator {

  namespace test_client {

    UserSimulator::UserSimulator(const core::UserIdentifier& id, const std::string& name):
      userInformation(id, name), connected(false)
    {

    }

    void UserSimulator::changeName(const std::string& name)
    {
      userInformation.name = name;
    }

    bool UserSimulator::isConnected() const
    {
      return connected;
    }

    void UserSimulator::setConnected(bool connected)
    {
      this->connected = connected;
    }

    const std::string& UserSimulator::getName() const
    {
      return userInformation.name;
    }
  }
  }
