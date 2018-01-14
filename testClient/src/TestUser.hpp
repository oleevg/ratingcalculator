/*
 * TestUser.hpp
 *
 *  Created on: 1/8/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_TESTUSER_HPP
#define RATINGCALCULATOR_TESTUSER_HPP

#include <string>

#include <core/Types.hpp>

namespace rating_calculator {

  namespace test_client {

    class TestUser {
      public:
        TestUser(const core::UserIdentifier& id, const std::string& name);

        void setConnected(bool connected);
        void changeName(const std::string& name);
        const std::string& getName() const;
        bool isConnected() const;

      private:
        core::UserInformation userInformation;
        bool connected;
    };

  }
}


#endif //RATINGCALCULATOR_TESTUSER_HPP
