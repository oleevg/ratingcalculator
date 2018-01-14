/*
 * TimeHelper.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <ctime>
#include <ratio>

#include <core/TimeHelper.hpp>

namespace rating_calculator {

  namespace core {

    typedef std::chrono::duration<int, std::ratio<3600*24>> DurationInDays;

    TimePoint TimeHelper::getPreviousWeekDay(const TimePoint& timePoint, WeekDay weekDay)
    {
      auto tTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

      std::tm* tmTime = std::gmtime(&tTime);

      int shiftDays = tmTime->tm_wday - static_cast<uint8_t>(weekDay);
      shiftDays = (shiftDays > 0) ? shiftDays : shiftDays + 7;

      std::tm resultTmTime = *tmTime;
      resultTmTime.tm_hour = 0;
      resultTmTime.tm_min = 0;
      resultTmTime.tm_sec = 0;
      resultTmTime.tm_mday -= shiftDays;

      auto result = timegm(&resultTmTime);

      return std::chrono::system_clock::from_time_t(result);
    }

    TimePoint TimeHelper::getNextWeekDay(const TimePoint& timePoint, WeekDay weekDay)
    {
      auto result = getPreviousWeekDay(timePoint, weekDay) + 7 * DurationInDays(1);

      return result;
    }
  }

}
