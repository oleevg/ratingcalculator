/*
 * TimeHelper_test.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#define BOOST_TEST_MODULE "Time helper test module"

#include <core/TimeHelper.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TimeHelper)

  namespace core = rating_calculator::core;

  BOOST_AUTO_TEST_CASE(GetPrevious_fixed_date_true)
  {
    // Sun Jan 14 2018 ->
    std::tm tmTime;
    tmTime.tm_sec = 0;
    tmTime.tm_min = 0;
    tmTime.tm_hour = 0;

    tmTime.tm_mon = 0;
    tmTime.tm_mday = 14;
    tmTime.tm_year = 118;

    // Mon Jan 08 2018
    std::tm tmExpectedTime;
    tmExpectedTime.tm_sec = 0;
    tmExpectedTime.tm_min = 0;
    tmExpectedTime.tm_hour = 0;

    tmExpectedTime.tm_mon = 0;
    tmExpectedTime.tm_mday = 8;
    tmExpectedTime.tm_year = 118;

    auto timePoint = core::TimeHelper::getPreviousWeekDay(std::chrono::system_clock::from_time_t(timegm(&tmTime)), core::TimeHelper::WeekDay::Monday);

    std::time_t resultTime = std::chrono::system_clock::to_time_t(timePoint);
    std::time_t expectedResultTime = timegm(&tmExpectedTime);

    BOOST_REQUIRE( resultTime == expectedResultTime);
  }

BOOST_AUTO_TEST_SUITE_END()

