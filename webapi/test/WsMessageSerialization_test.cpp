/*
 * WsMessageSerialization_test.cpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#define BOOST_TEST_MODULE "WsMessageSerialization test module"

#include <iostream>

#include <boost/test/unit_test.hpp>

#include <webapi/serialization/JsonSerializerWsMessage.hpp>
#include <webapi/serialization/JsonDeserializerWsMessage.hpp>

BOOST_AUTO_TEST_SUITE (WsMessageJsonSerializer)

  namespace transport = rating_calculator::webapi::transport;
  namespace serialization = rating_calculator::webapi::serialization;

  bool operator==(const transport::WsMessage& lhs, const transport::WsMessage& rhs)
  {
    std::cout << "WsMessage comparison" << std::endl;
    return (lhs.getType() == rhs.getType() && lhs.getId() == rhs.getId());
  }

  bool operator==(const transport::WsError& lhs, const transport::WsError& rhs)
  {
    bool base = static_cast<const transport::WsMessage&>(lhs) == static_cast<const transport::WsMessage&>(rhs);
    bool derived = (lhs.getErrorCode() == rhs.getErrorCode() && lhs.getErrorMessage() == rhs.getErrorMessage());

    return (base && derived);
  }

  template <class T>
  bool operator==(const transport::WsData<T>& lhs, const transport::WsData<T>& rhs)
  {
    std::cout << "WsData comparison" << std::endl;
    bool base = static_cast<const transport::WsMessage&>(lhs) == static_cast<const transport::WsMessage&>(rhs);
    bool derived = (*lhs.getData() == *rhs.getData());

    return (base && derived);
  }

  BOOST_AUTO_TEST_CASE(Serialize_Deserialize_equal_true)
  {
    // rating_calculator::webapi::transport::WsError
    rating_calculator::webapi::transport::WsError wsError(123, "Base error", 0);

    auto tree = serialization::JsonSerializer<transport::WsError>::Serialize(wsError);

    {
      std::stringstream stringstream;
      boost::property_tree::write_json(stringstream, tree);
      std::cout << "Serialized: " << stringstream.str() << std::endl;
    }

    auto wsErrorDeserialized = serialization::JsonDeserializer<decltype(wsError)>::Parse(tree);

    BOOST_REQUIRE(wsError == wsErrorDeserialized);

    // transport::WsData<transport::Message<rating_calculator::core::UserInformation>>
    transport::WsData<transport::Message<rating_calculator::core::UserInformation>> wsData(123, transport::MessageType::UserRegistered, 1234, "user");

    tree = serialization::JsonSerializer<decltype(wsData)>::Serialize(wsData);

    {
      std::stringstream stringstream;
      boost::property_tree::write_json(stringstream, tree);
      std::cout << "Serialized: " << stringstream.str() << std::endl;
    }

    auto wsDataDeserialized = serialization::JsonDeserializer<decltype(wsData)>::Parse(tree);

    BOOST_REQUIRE(wsData == wsDataDeserialized);

    // transport::WsData<transport::Message<rating_calculator::core::DealInformation>>
    transport::WsData<transport::Message<rating_calculator::core::DealInformation>> wsDealInformation(124, transport::MessageType::UserDealWon, 1234, 777, 123.456);

    tree = serialization::JsonSerializer<decltype(wsDealInformation)>::Serialize(wsDealInformation);

    {
      std::stringstream stringstream;
      boost::property_tree::write_json(stringstream, tree);
      std::cout << "Serialized: " << stringstream.str() << std::endl;
    }

    auto wsDealInformationDeserialized = serialization::JsonDeserializer<decltype(wsDealInformation)>::Parse(tree);

    BOOST_REQUIRE(wsDealInformation == wsDealInformationDeserialized);
  }

BOOST_AUTO_TEST_SUITE_END()

