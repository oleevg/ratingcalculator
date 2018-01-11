/*
 * EnumConverter_test.cpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#define BOOST_TEST_MODULE "JsonSerialization test module"

#include <string>
#include <cstdint>

#include <boost/test/unit_test.hpp>

#include <core/EnumConverter.hpp>

enum class TestEnum : uint8_t {
    Value1 = 0,
    Value2 = 1
};

namespace rating_calculator {

  namespace core {

    template<>
    rating_calculator::core::EnumConverter<TestEnum>::EnumConverter()
    {
      addConversion(TestEnum::Value1, "value1");
      addConversion(TestEnum::Value2, "value2");
    }

  }

}

BOOST_AUTO_TEST_SUITE(EnumConverter)


  BOOST_AUTO_TEST_CASE(EnumToString_equal_true)
  {
    const auto& enumConverter = rating_calculator::core::EnumConverter<TestEnum>::get_const_instance();

    const std::string& value1 = enumConverter.toString(TestEnum::Value1);
    const std::string& value2 = enumConverter.toString(TestEnum::Value2);

    BOOST_REQUIRE(value1 == "value1");
    BOOST_REQUIRE(value2 == "value2");
  }

//  BOOST_AUTO_TEST_CASE(StringToEnum_equal_true)
//  {
//    const auto& enumConverter = rating_calculator::core::EnumConverter<TestEnum>::get_const_instance();
//
//    auto value1 = enumConverter.toEnum("value1");
//    auto value2 = enumConverter.toEnum("value2");
//
//    BOOST_REQUIRE(value1 == TestEnum::Value1);
//    BOOST_REQUIRE(value2 == TestEnum::Value2);
//  }
//
//  BOOST_AUTO_TEST_CASE(StringToEnumInCase_equal_true)
//  {
//    const auto& enumConverter = rating_calculator::core::EnumConverter<TestEnum>::get_const_instance();
//
//    auto value1 = enumConverter.toEnum("Value1");
//    auto value2 = enumConverter.toEnum("vALUE2");
//
//    BOOST_REQUIRE(value1 == TestEnum::Value1);
//    BOOST_REQUIRE(value2 == TestEnum::Value2);
//  }

BOOST_AUTO_TEST_SUITE_END()

