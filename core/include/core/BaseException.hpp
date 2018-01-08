/*
 * BaseException.hpp
 *
 *  Created on: 10/1/17
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef C_BASEEXCEPTION_HPP
#define C_BASEEXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace rating_calculator {

  namespace core {

    /**
     * @brief Applications's base exception class.
     */
    class BaseException : public std::exception {

      public:
        BaseException(const std::string& errorMessage, const std::string& header = "Base error occurred: ");

        const char* what() const noexcept override;

      private:
        std::string errorMessage_;
    };

  }

}


#endif //C_BASEEXCEPTION_HPP
