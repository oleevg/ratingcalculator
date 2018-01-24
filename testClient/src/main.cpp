/*
 * main.cpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <string>
#include <memory>
#include <iostream>
#include <thread>
#include <atomic>
#include <random>

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <core/ulog.h>

#include <webapi/websockets/client_ws.hpp>
#include <webapi/transport/WsProtocol.hpp>

#include "RequestGenerator.hpp"

typedef SimpleWeb::SocketClient<SimpleWeb::WS> WsClient;

namespace core = rating_calculator::core;
namespace transport  = rating_calculator::webapi::transport;
namespace options = boost::program_options;

int main(int argc, const char* argv[])
{
  /* Default parameters definition */
  const std::string addressDefault = "localhost";
  const int portDefault = 88888;
  const int requestTimeoutDefault = 1;
  const size_t nUsersDefault = 100;

  options::options_description optionDescription((boost::format("Usage: %s [options]... \nOptions") % argv[0]).str());

  std::string address = addressDefault;
  int port = portDefault;
  int requestTimeout = requestTimeoutDefault;
  size_t nUsers = nUsersDefault;

  optionDescription.add_options()
          ("address,a", options::value<std::string>(&address)->default_value(addressDefault), "The server address to connect to.")
          ("port,p", options::value<int>(&port)->default_value(portDefault), "The port number to connect to.")
          ("timeout,t", options::value<int>(&requestTimeout)->default_value(requestTimeoutDefault), "Timeout in seconds to send generated test requests.")
          ("users,u", options::value<size_t>(&nUsers)->default_value(nUsersDefault), "Maximum number of test users.")
          ("help,h", "As it says.");

  options::variables_map variableMap;

  options::store(options::parse_command_line(argc, argv, optionDescription), variableMap);
  options::notify(variableMap);

  if(variableMap.count("help"))
  {
    std::cout << optionDescription << "\n";
    exit(0);
  }

  std::atomic<bool> stopped(false);

  std::string wsUri = (boost::format("%s:%d/rating") % address % port).str();
  WsClient client(wsUri);

  std::shared_ptr<WsClient::Connection> clientConnection;

  transport::WsProtocol<WsClient> protocol;
  protocol.start();

  client.on_message = [&protocol](std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::Message> message) {
    core::BaseMessage::Ptr baseMessage = protocol.parseMessage(message, connection);
  };

  client.on_open = [&clientConnection, wsUri](std::shared_ptr<WsClient::Connection> connection) {
    mdebug_info("Connected to '%s'.", wsUri.c_str());
    clientConnection = connection;
  };

  client.on_close = [](std::shared_ptr<WsClient::Connection> /*connection*/, int status, const std::string & /*reason*/) {
    mdebug_info("Client: Closed connection with status code '%d'.", status);
  };

  // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
  client.on_error = [&stopped, &client](std::shared_ptr<WsClient::Connection> connection, const SimpleWeb::error_code &ec) {
    mdebug_error("Error in connection. Error: %s (%d).", ec.message().c_str(), ec);

    client.stop();
    stopped.store(true);
  };


  std::thread clientThread([&client]() {
    client.start();
  });


  while (!clientConnection && !stopped.load())
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  rating_calculator::test_client::RequestGenerator requestGenerator(nUsers);

  std::thread usersRegisterThread([nUsers, clientConnection, &requestGenerator, &protocol, &stopped]() {
    std::mt19937 rg{std::random_device{}()};
    std::uniform_int_distribution<size_t> pickTimeout(1, 1000);

    while (requestGenerator.getRegisteredUsersNumber() != nUsers && !stopped.load())
    {
      auto userRegisteredMessage = requestGenerator.generateUserRegisteredMessage();
      protocol.sendMessage(userRegisteredMessage, clientConnection);

      std::this_thread::sleep_for(std::chrono::milliseconds(pickTimeout(rg)));
    }

  });

  // Requests generation loop
  while(!stopped.load())
  {
    requestGenerator.waitForUsersToRegister();

    auto message = requestGenerator.generateUserCommonMessage();
    protocol.sendMessage(message, clientConnection);

    std::this_thread::sleep_for(std::chrono::seconds(requestTimeout));
  }

  protocol.stop();

  usersRegisterThread.join();
  clientThread.join();

  return 0;
}

