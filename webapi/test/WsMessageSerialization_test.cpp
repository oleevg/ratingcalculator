/*
 * WsMessageSerialization_test.cpp
 *
 *  Created on: 1/6/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#define BOOST_TEST_MODULE "WsMessageSerialization test module"

#include <boost/test/unit_test.hpp>

#include <webapi/serialization/JsonSerializerWsMessage.hpp>
#include <webapi/serialization/JsonDeserializerWsMessage.hpp>

BOOST_AUTO_TEST_SUITE (WsMessageJsonSerializer)

  namespace core = rating_calculator::core;
  namespace transport = rating_calculator::webapi::transport;
  namespace serialization = rating_calculator::webapi::serialization;

  bool operator==(const transport::WsMessage& lhs, const transport::WsMessage& rhs)
  {
    return (lhs.getType() == rhs.getType() && lhs.getId() == rhs.getId());
  }

  bool operator==(const transport::WsError& lhs, const transport::WsError& rhs)
  {
    bool base = static_cast<const transport::WsMessage&>(lhs) == static_cast<const transport::WsMessage&>(rhs);
    bool derived = (lhs.getErrorCode() == rhs.getErrorCode() && lhs.getErrorMessage() == rhs.getErrorMessage());

    return (base && derived);
  }

  template <class T>
  bool operator==(const transport::WsData& lhs, const transport::WsData& rhs)
  {
    bool base = static_cast<const transport::WsMessage&>(lhs) == static_cast<const transport::WsMessage&>(rhs);

    auto lData = std::static_pointer_cast<core::Message <T>>(lhs.getData());
    auto rData = std::static_pointer_cast<core::Message <T>>(rhs.getData());

    bool derived = operator==<T>(*lData, *rData);

    return (base && derived);
  }

  BOOST_AUTO_TEST_CASE(Serialize_Deserialize_equal_true)
  {
    // rating_calculator::webapi::transport::WsError
    rating_calculator::webapi::transport::WsError wsError(123, "Base error", 0);

    auto tree = serialization::JsonSerializer<transport::WsError>::Serialize(wsError);
    auto wsErrorDeserialized = serialization::JsonDeserializer<decltype(wsError)>::Parse(tree);

    BOOST_REQUIRE(wsError == wsErrorDeserialized);

    // transport::WsData<core::Message<rating_calculator::core::UserInformation>>
    transport::WsData wsData(123, std::make_shared<rating_calculator::core::Message<rating_calculator::core::UserInformation>>(rating_calculator::core::MessageType::UserRegistered, 1234, "user"));

    tree = serialization::JsonSerializer<decltype(wsData)>::Serialize(wsData);

    auto wsDataDeserialized = serialization::JsonDeserializer<decltype(wsData)>::Parse(tree);

    BOOST_REQUIRE(wsData == wsDataDeserialized);

    // transport::WsData<core::Message<rating_calculator::core::DealInformation>>
    transport::WsData wsDealInformation(124, std::make_shared<rating_calculator::core::Message<rating_calculator::core::DealInformation>>(rating_calculator::core::MessageType::UserDealWon, 1234, 777, 123.456));
    tree = serialization::JsonSerializer<decltype(wsDealInformation)>::Serialize(wsDealInformation);

    auto wsDealInformationDeserialized = serialization::JsonDeserializer<decltype(wsDealInformation)>::Parse(tree);

    BOOST_REQUIRE(wsDealInformation == wsDealInformationDeserialized);
  }

BOOST_AUTO_TEST_SUITE_END()

