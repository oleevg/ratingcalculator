/*
 * Application.hpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_APPLICATION_HPP
#define RATINGCALCULATOR_APPLICATION_HPP

#include "ApplicationService.hpp"

namespace rating_calculator {

  namespace service {

    /**
     * @brief Application main class.
     * @detailed Provides application arguments handling.
     */
    class Application {
        /**
         * @brief Starts application's main cycle.
         */
      public:
        int run(int argc, const char** argv);

      private:
        void parseArguments(int argc, const char** argv);

      private:
        ApplicationService::Ptr appService;
    };

  }

}


#endif //RATINGCALCULATOR_APPLICATION_HPP
