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
#include <chrono>

#include <webapi/websockets/client_ws.hpp>
#include <webapi/transport/WsProtocol.hpp>

#include "RequestGenerator.hpp"

typedef SimpleWeb::SocketClient<SimpleWeb::WS> WsClient;

namespace core = rating_calculator::core;
namespace transport  = rating_calculator::webapi::transport;

int main(int argc, const char* argv[])
{
  WsClient client("localhost:88888/rating");
  std::shared_ptr<WsClient::Connection> clientConnection;

  transport::WsProtocol protocol(3, 3);

  client.on_message = [&protocol](std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::Message> message) {
//    std::cout << "Client: Message received: \"" << message->string() << "\"" << std::endl;

    transport::BaseMessage::Ptr baseMessage = protocol.parseMessage<WsClient>(message, connection);
  };

  client.on_open = [&clientConnection](std::shared_ptr<WsClient::Connection> connection) {
    std::cout << "Client: Opened connection" << std::endl;
    clientConnection = connection;
  };

  client.on_close = [](std::shared_ptr<WsClient::Connection> /*connection*/, int status, const std::string & /*reason*/) {
    std::cout << "Client: Closed connection with status code " << status << std::endl;
  };

  // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
  client.on_error = [](std::shared_ptr<WsClient::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
    std::cout << "Client: Error: " << ec << ", error message: " << ec.message() << std::endl;
  };


  std::thread clientThread([&client]()
                           {
                             client.start();
                           });

  std::this_thread::sleep_for(std::chrono::seconds(3));

  rating_calculator::test_client::RequestGenerator requestGenerator;

  while(true)
  {
    auto message = requestGenerator.generateMessage();

    if(!message)
    {
      continue;
    }

    if (message->getType() == rating_calculator::webapi::transport::MessageType::UserConnected)
    {
      auto userConnectedMessage = std::static_pointer_cast<transport::Message<core::UserIdInformation>>(message);

      std::cout << "Connection: " << clientConnection << std::endl;
      protocol.sendMessage<core::UserIdInformation, WsClient>(*userConnectedMessage, clientConnection);
    }
    else if (message->getType() == rating_calculator::webapi::transport::MessageType::UserRegistered)
    {
      auto userRegisteredMessage = std::static_pointer_cast<transport::Message<core::UserInformation>>(message);

      std::cout << "Connection: " << clientConnection << std::endl;
      protocol.sendMessage<core::UserInformation, WsClient>(*userRegisteredMessage, clientConnection);
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
  }

  clientThread.join();

  return 0;
}

