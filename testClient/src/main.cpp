/*
 * main.cpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <core/ITransportClient.hpp>
#include <core/ulog.h>

#include <webapi/transport/WsTransportClient.hpp>
#include <grpcapi/GrpcTransportClient.hpp>

#include "RequestGenerator.hpp"

namespace options = boost::program_options;

int main(int argc, const char* argv[])
{
  const std::string addressDefault = "localhost";
  const int portDefault = 88888;
  const int requestTimeoutDefault = 1;
  const std::size_t nUsersDefault = 100;
  const std::string transportDefault = "ws";

  options::options_description optionDescription((boost::format("Usage: %s [options]... \nOptions") % argv[0]).str());

  std::string address = addressDefault;
  int port = portDefault;
  int requestTimeout = requestTimeoutDefault;
  std::size_t nUsers = nUsersDefault;
  std::string transportName = transportDefault;

  optionDescription.add_options()("address,a", options::value<std::string>(&address)->default_value(addressDefault),
                                  "Server address.")("port,p", options::value<int>(&port)->default_value(portDefault),
                                                     "Server port.")(
      "timeout,t", options::value<int>(&requestTimeout)->default_value(requestTimeoutDefault),
      "Seconds between generated requests.")(
      "users,u", options::value<std::size_t>(&nUsers)->default_value(nUsersDefault), "Number of simulated users.")(
      "transport", options::value<std::string>(&transportName)->default_value(transportDefault),
      "Transport to use: ws | grpc.")("help,h", "Show this help message.");

  options::variables_map variableMap;
  options::store(options::parse_command_line(argc, argv, optionDescription), variableMap);
  options::notify(variableMap);

  if (variableMap.count("help"))
  {
    std::cout << optionDescription << "\n";
    return 0;
  }

  // Build the transport
  rating_calculator::core::ITransportClient::Ptr transport;
  if (transportName == "ws")
  {
    transport = std::make_shared<rating_calculator::webapi::transport::WsTransportClient>(address, port);
  }
  else if (transportName == "grpc")
  {
    transport = std::make_shared<rating_calculator::grpcapi::GrpcTransportClient>(address, port);
  }
  else
  {
    std::cerr << "Unknown transport '" << transportName << "'. Valid values: ws, grpc.\n";
    return 1;
  }

  transport->setMessageHandler(
      [](rating_calculator::core::BaseMessage::Ptr /*msg*/)
      {
        // Rating updates from the server are received here.
        // In a real client this would update a UI or trigger re-pricing.
      });

  transport->connect();

  if (!transport->waitForConnection(std::chrono::seconds(10)))
  {
    mdebug_error("Failed to connect to %s:%d via %s.", address.c_str(), port, transportName.c_str());
    return 1;
  }

  mdebug_info("Connected to %s:%d via %s.", address.c_str(), port, transportName.c_str());

  rating_calculator::test_client::RequestGenerator requestGenerator(nUsers);

  std::thread usersRegisterThread(
      [nUsers, &requestGenerator, &transport]()
      {
        std::mt19937 rg{std::random_device{}()};
        std::uniform_int_distribution<std::size_t> pickTimeout(1, 1000);

        while (requestGenerator.getRegisteredUsersNumber() != nUsers)
        {
          auto message = requestGenerator.generateUserRegisteredMessage();
          transport->sendMessage(message);
          std::this_thread::sleep_for(std::chrono::milliseconds(pickTimeout(rg)));
        }
      });

  while (true)
  {
    requestGenerator.waitForUsersToRegister();

    auto message = requestGenerator.generateUserCommonMessage();
    if (message)
    {
      transport->sendMessage(message);
    }

    std::this_thread::sleep_for(std::chrono::seconds(requestTimeout));
  }

  usersRegisterThread.join();
  transport->disconnect();

  return 0;
}
