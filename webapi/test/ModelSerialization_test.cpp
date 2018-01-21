/*
 * serialization_test.cpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#define BOOST_TEST_MODULE "JsonSerialization test module"


#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <webapi/serialization/JsonSerializerModel.hpp>
#include <webapi/serialization/JsonDeserializerModel.hpp>
#include <webapi/serialization/SerializerException.hpp>

BOOST_AUTO_TEST_SUITE (JsonSerializer)

  namespace core = rating_calculator::core;
  namespace serialization = rating_calculator::webapi::serialization;

  template <class T>
  using JsSerializer = serialization::JsonSerializer<T>;

  template <class T>
  using JsDeserializer = serialization::JsonDeserializer<T>;

  BOOST_AUTO_TEST_CASE(Serialize_Deserialize_equal_true)
  {
    {
      // core::UserInformation
      core::UserInformation userInformation{12345, "user"};

      auto tree = JsSerializer<decltype(userInformation)>::Serialize(userInformation);

//      {
//        std::stringstream stringstream;
//        boost::property_tree::write_json(stringstream, tree);
//        std::cout << "Serialized: " << stringstream.str() << std::endl;
//      }

      core::UserInformation userInformationDeserialized = JsDeserializer<core::UserInformation>::Parse(tree);

      BOOST_REQUIRE(userInformation == userInformationDeserialized);
    }

    {
      // core::Message<core::UserInformation>
      core::Message<core::UserInformation> message(core::MessageType::UserRegistered, 12345, "user");

      auto tree = serialization::JsonSerializer<decltype(message)>::Serialize(message);

//      {
//        std::stringstream stringstream;
//        boost::property_tree::write_json(stringstream, tree);
//        std::cout << "Serialized: " << stringstream.str() << std::endl;
//      }

      auto messageDeserialized = serialization::JsonDeserializer<decltype(message)>::Parse(tree);

      BOOST_REQUIRE(message == messageDeserialized);
    }

    {
      // core::Message<core::UserRelativeRating>
      core::UserInformation userInformation(12345, "user");
      core::UserPosition userPosition(userInformation, 21, 12345.67);

      core::UserPositionsCollection headPositions;
      headPositions.reserve(10);
      for (size_t pos = 1; pos < 11; ++pos)
      {
        headPositions.emplace_back(userInformation, pos, 12345.67);
      }

      core::UserPositionsCollection highPositions;
      headPositions.reserve(10);
      for (size_t pos = 11; pos < 21; ++pos)
      {
        highPositions.emplace_back(userInformation, pos, 12345.67);
      }

      core::UserPositionsCollection lowPositions;
      lowPositions.reserve(10);
      for (size_t pos = 22; pos < 32; ++pos)
      {
        lowPositions.emplace_back(userInformation, pos, 12345.67);
      }

      core::UserRelativeRating userRelativeRating(userPosition, headPositions, highPositions, lowPositions);
      core::Message<core::UserRelativeRating> message(core::MessageType::UserRelativeRating, userRelativeRating);

      auto tree = serialization::JsonSerializer<decltype(message)>::Serialize(message);

      {
        std::stringstream stringstream;
        boost::property_tree::write_json(stringstream, tree);
        std::cout << "Serialized: " << stringstream.str() << std::endl;
      }

      auto messageDeserialized = serialization::JsonDeserializer<decltype(message)>::Parse(tree);

      BOOST_REQUIRE(message == messageDeserialized);
    }
  }

  BOOST_AUTO_TEST_CASE(Deserialize_empty_throw)
  {
    boost::property_tree::ptree tree;

    BOOST_REQUIRE_THROW(JsDeserializer<core::UserInformation>::Parse(tree), serialization::SerializerException);
  }

  BOOST_AUTO_TEST_CASE(Vector_Serialize_Deserialize_equal_true)
  {
    std::vector<core::UserInformation> users = {{1234, "user1"}, {12345, "user2"}};

    auto child = JsSerializer<decltype(users)>::Serialize(users);
    boost::property_tree::ptree tree;
    tree.add_child("users", child);

//    std::stringstream stringstream;
//    boost::property_tree::write_json(stringstream, tree);
//    std::cout << "Serialized: " << stringstream.str() << std::endl;

    auto usersDeserialized = JsDeserializer<decltype(users)>::Parse(tree.get_child("users"));

    BOOST_REQUIRE(users.size() == usersDeserialized.size());

    for (size_t i = 0; i < users.size(); ++i)
    {
      BOOST_REQUIRE(users[i] == usersDeserialized[i]);
    }
  }

BOOST_AUTO_TEST_SUITE_END()

