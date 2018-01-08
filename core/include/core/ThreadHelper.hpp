/*
 * ThreadHelper.hpp
 *
 *  Created on: 10/4/17
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef C_THREADMACRO_HPP
#define C_THREADMACRO_HPP

#include <sstream>
#include <string>
#include <thread>

namespace rating_calculator {

  namespace core {

    /**
     * @brief Helper class to work with std::thread identifier.
     */
    class ThreadHelper {
      public:
        static std::string threadIdToString()
        {
          std::stringstream stringStream;
          stringStream << std::this_thread::get_id();

          return stringStream.str();
        }

        static size_t threadIdToInt()
        {
          std::string strId = threadIdToString();

          return std::stol(strId);
        }
    };

  }

}


#endif //C_THREADMACRO_HPP
