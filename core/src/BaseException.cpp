/*
 * BaseException.cpp
 *
 *  Created on: 10/1/17
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <core/BaseException.hpp>

namespace rating_calculator {

  namespace core {

    BaseException::BaseException(const std::string& errorMessage, const std::string& header) :
    errorMessage_(header + errorMessage)
    {

    }

    const char* BaseException::what() const noexcept
    {
      return errorMessage_.c_str();
    }
  }

}

