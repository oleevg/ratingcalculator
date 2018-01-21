/*
 * VolatileKeyAssocVector_test.cpp
 *
 *  Created on: 1/16/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#define BOOST_TEST_MODULE "Volatile key associative vector test module"

#include <vector>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include <core/MultiKeyVolatileContainer.hpp>


struct UserData {
  UserData(uint64_t _id, float _amount):
          id(_id), amount(_amount)
  {

  }

  uint64_t id;
  float amount;
};

struct FloatComparator {
  bool operator()(float lhs, float rhs)
  {

  }
};

typedef  rating_calculator::core::MultiKeyVolatileContainer<uint64_t, float, UserData, &UserData::id, &UserData::amount> MultiKeyContainer;

struct MultiKeyVolatileContainerFixture {
  MultiKeyVolatileContainerFixture():
          multiKeyContainer(UINT16_MAX)
  {
    std::vector<UserData> users;
    users.push_back(UserData(5, 0.4));
    users.push_back(UserData(9, 0.0));
    users.push_back(UserData(1, 0.8));
    users.push_back(UserData(0, 0.9));
    users.push_back(UserData(2, 0.7));
    users.push_back(UserData(4, 0.5));
    users.push_back(UserData(6, 0.3));
    users.push_back(UserData(7, 0.2));
    users.push_back(UserData(3, 0.6));
    users.push_back(UserData(8, 0.1));

    for (const auto& user: users)
    {
      multiKeyContainer.insert(user);
    }
  }

  MultiKeyContainer multiKeyContainer;

};

BOOST_FIXTURE_TEST_SUITE(MultiKeyVolatileContainer, MultiKeyVolatileContainerFixture)

  BOOST_AUTO_TEST_CASE(Insert_item_must_be_sorted_true)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      size_t position = i;
      auto positionedData = multiKeyContainer.findWithPosition(position);

      std::cout << "result: " << positionedData.value.amount << ", expected: " << 0.9 - i*0.1 << std::endl;

      BOOST_REQUIRE(positionedData.position == position);
      BOOST_TEST(positionedData.value.amount == (0.9 - position*0.1),  boost::test_tools::tolerance(0.001));
    }
  }

  BOOST_AUTO_TEST_CASE(Update_item_must_be_sorted_true)
  {
    size_t position = 8;
    multiKeyContainer.insert(UserData(8, 0.9));
    auto positionedData = multiKeyContainer.findWithPosition(position);

    BOOST_REQUIRE(positionedData.position == 0);
    BOOST_TEST(positionedData.value.amount == (0.9 + 0.1), boost::test_tools::tolerance(0.001));
  }

  BOOST_AUTO_TEST_CASE(GetHeadPositions_compare_true)
  {
    size_t nPositions = 9;
    auto positionedDataContainer = multiKeyContainer.getHeadPositions(nPositions);

    for (size_t i = 0; i < nPositions; ++i)
    {
      BOOST_REQUIRE(positionedDataContainer[i].position == i);
      BOOST_TEST(positionedDataContainer[i].value.amount == (0.9 - i*0.1), boost::test_tools::tolerance(0.001));
    }
  }

  BOOST_AUTO_TEST_CASE(GetHighPositions_compare_true)
  {
    size_t nPositions = 5;
    size_t position = 4;
    auto positionedDataContainer = multiKeyContainer.getHighPositions(position, nPositions);

    BOOST_REQUIRE(positionedDataContainer.size() == 4);

    for (size_t i = 0; i < nPositions && i < positionedDataContainer.size(); ++i)
    {
      BOOST_REQUIRE(positionedDataContainer[i].position == i);
      BOOST_TEST(positionedDataContainer[i].value.amount == (0.9 - i*0.1),  boost::test_tools::tolerance(0.001));
    }
  }

  BOOST_AUTO_TEST_CASE(GetLowPositions_compare_true)
  {
    size_t nPositions = 5;
    size_t position = 4;
    auto positionedDataContainer = multiKeyContainer.getLowPositions(position, nPositions);

    BOOST_REQUIRE(positionedDataContainer.size() == 5);

    for (size_t i = 0; i < nPositions && i < positionedDataContainer.size(); ++i)
    {
      size_t shift = i + position + 1;
      std::cout << "result: " << positionedDataContainer[i].value.amount << ", expected: " << (0.9 - shift*0.1) << std::endl;
      BOOST_REQUIRE(positionedDataContainer[i].position == shift);
      BOOST_TEST(positionedDataContainer[i].value.amount == (0.9 - shift*0.1),  boost::test_tools::tolerance(0.001));
    }
  }

BOOST_AUTO_TEST_SUITE_END()


