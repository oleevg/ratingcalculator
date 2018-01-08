/*
 * RequestGenerator.hpp
 *
 *  Created on: 1/8/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_REQUESTGENERATOR_HPP
#define RATINGCALCULATOR_REQUESTGENERATOR_HPP

#include <vector>

#include <webapi/transport/Model.hpp>

#include "UserSimulator.hpp"

namespace rating_calculator {

  namespace test_client {

    class RequestGenerator {
      public:
        RequestGenerator();

        webapi::transport::BaseMessage::Ptr generateMessage();

      private:
        std::vector<UserSimulator> users;

        const size_t nUsersMax;
    };

  }

}


#endif //RATINGCALCULATOR_REQUESTGENERATOR_HPP
