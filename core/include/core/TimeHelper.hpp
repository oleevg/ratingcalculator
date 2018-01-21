/*
 * TimeHelper.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_TIMEHELPER_HPP
#define RATINGCALCULATOR_TIMEHELPER_HPP

#include <chrono>
#include <cstdint>
#include <string>

namespace rating_calculator {

  namespace core {

    typedef std::chrono::system_clock::time_point TimePoint;

    class TimeHelper {
      public:
        enum class WeekDay : uint8_t {
            Monday = 1,
            Tuesday = 2,
            Wednesday = 3,
            Thursday = 4,
            Friday = 5,
            Saturday = 6,
            Sunday = 0
        };

      public:
        static TimePoint getPreviousWeekDay(const TimePoint& timePoint, WeekDay weekDay);
        static TimePoint getNextWeekDay(const TimePoint& timePoint, WeekDay weekDay);

        static std::string toString(const TimePoint& timePoint);
    };

  }

}


#endif //RATINGCALCULATOR_TIMEHELPER_HPP
