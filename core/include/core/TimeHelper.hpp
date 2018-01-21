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

    /**
     * @brief Helper class to work with time and date.
     */
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
        /**
         * @brief Provides information about the previous week day in relation to the specific time point.
         * @param timePoint Specific time point to find date in relation to.
         * @param weekDay Specific week day to find.
         * @return Actual time point corresponding to the previous week day.
         */
        static TimePoint getPreviousWeekDay(const TimePoint& timePoint, WeekDay weekDay);

        /**
          * @brief Provides information about the next week day in relation to the specific time point.
          * @param timePoint Specific time point to find date in relation to.
          * @param weekDay Specific week day to find.
          * @return Actual time point corresponding to the next week day.
          */
        static TimePoint getNextWeekDay(const TimePoint& timePoint, WeekDay weekDay);

        /**
         * @brief Provides string representation for date.
         * @param timePoint Time point.
         * @return Actual string representation.
         */
        static std::string toString(const TimePoint& timePoint);
    };

  }

}


#endif //RATINGCALCULATOR_TIMEHELPER_HPP
