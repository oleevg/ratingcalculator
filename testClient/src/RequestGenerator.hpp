/*
 * RequestGenerator.hpp
 *
 *  Created on: 1/8/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_REQUESTGENERATOR_HPP
#define RATINGCALCULATOR_REQUESTGENERATOR_HPP

#include <vector>
#include <mutex>
#include <condition_variable>

#include <core/Model.hpp>

#include "UserSimulator.hpp"

namespace rating_calculator {

  namespace test_client {

    class RequestGenerator {
      public:
        RequestGenerator(size_t usersNumber);

        core::BaseMessage::Ptr generateUserCommonMessage();
        core::BaseMessage::Ptr generateUserRegistedMessage();

        size_t getRegisteredUsersNumber() const;

        void waitForUsersToRegister();

      private:
        std::vector<UserSimulator> users;
        mutable std::mutex usersMutex;
        std::condition_variable usersCondVar;

        const size_t nUsersMax;
    };

  }

}


#endif //RATINGCALCULATOR_REQUESTGENERATOR_HPP
