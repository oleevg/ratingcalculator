/*
 * SerializerException.hpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_SERIALIZEREXCEPTION_HPP
#define RATINGCALCULATOR_SERIALIZEREXCEPTION_HPP

#include <core/BaseException.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace serialization {

      /**
       * @brief Serializer specific exception type.
       */
      class SerializerException : public core::BaseException {
        public:
          SerializerException(const std::string& errorMessage):
                  core::BaseException(errorMessage, "Serializer exception occurred: ")
          { }
      };

    }

  }

}

#endif //RATINGCALCULATOR_SERIALIZEREXCEPTION_HPP
