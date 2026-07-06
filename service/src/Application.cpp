/*
 * Application.cpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <string>
#include <iostream>
#include <chrono>

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <core/BaseException.hpp>
#include <core/ulog.h>
#include <core/ThreadHelper.hpp>

#include <webapi/transport/WsTransportServer.hpp>
#include <grpcapi/GrpcTransportServer.hpp>
#include <kafkaapi/KafkaTransportServer.hpp>
#include <rabbitapi/RabbitTransportServer.hpp>

#include "Application.hpp"

namespace rating_calculator {

  namespace service {

    int Application::run(int argc, const char** argv)
    {
      int rc = 0;

      try
      {
        parseArguments(argc, argv);
        rc = appService->run();
      } catch (const core::BaseException& exc)
      {
        mdebug_error(exc.what());
        rc = -1;
      } catch (const std::exception& exc)
      {
        mdebug_error("Unknown error occurred. %s", exc.what());
        rc = -1;
      }

      return rc;
    }

    void Application::parseArguments(int argc, const char** argv)
    {
      namespace options = boost::program_options;

      const int portDefault = 88888;
      const int periodDefault = 5;
      const size_t threadPoolSizeDefault = 5;
      const std::string transportDefault = "ws";

      options::options_description optionDescription(
          (boost::format("Usage: %s [options]... \nOptions") % argv[0]).str());

      int port = portDefault;
      int period = periodDefault;
      size_t threadPoolSize = threadPoolSizeDefault;
      std::string transportName = transportDefault;
      std::string kafkaBrokers = "localhost:9092";
      std::string kafkaTopic = "rating.events.in";
      std::string rabbitHost = "localhost";

      optionDescription.add_options()("port,p", options::value<int>(&port)->default_value(portDefault),
                                      "Port to listen on (ws/grpc).")(
          "period,t", options::value<int>(&period)->default_value(periodDefault), "Rating update period in seconds.")(
          "threads", options::value<size_t>(&threadPoolSize)->default_value(threadPoolSizeDefault),
          "Thread pool size (ws).")("transport",
                                    options::value<std::string>(&transportName)->default_value(transportDefault),
                                    "Transport: ws | grpc | kafka | rabbitmq.")(
          "kafka-brokers", options::value<std::string>(&kafkaBrokers)->default_value(kafkaBrokers),
          "Kafka broker list (kafka transport).")("kafka-topic",
                                                  options::value<std::string>(&kafkaTopic)->default_value(kafkaTopic),
                                                  "Kafka inbound topic (kafka transport).")(
          "rabbit-host", options::value<std::string>(&rabbitHost)->default_value(rabbitHost),
          "RabbitMQ host (rabbitmq transport).")("help,h", "Show this help message.");

      options::variables_map variableMap;
      options::store(options::parse_command_line(argc, argv, optionDescription), variableMap);
      options::notify(variableMap);

      if (variableMap.count("help"))
      {
        std::cout << optionDescription << "\n";
        exit(0);
      }

      if (period <= 0)
      {
        throw core::BaseException(
            "Negative or zero timeout values are not supported. Please provide a positive integer value.");
      }

      core::ITransportServer::Ptr transport;

      if (transportName == "ws")
      {
        transport = std::make_shared<webapi::transport::WsTransportServer>(port, threadPoolSize);
      }
      else if (transportName == "grpc")
      {
        transport = std::make_shared<grpcapi::GrpcTransportServer>(port);
      }
      else if (transportName == "kafka")
      {
        transport = std::make_shared<kafkaapi::KafkaTransportServer>(kafkaBrokers, kafkaTopic);
      }
      else if (transportName == "rabbitmq")
      {
        transport = std::make_shared<rabbitapi::RabbitTransportServer>(rabbitHost, port);
      }
      else
      {
        throw core::BaseException("Unknown transport '" + transportName +
                                  "'. Valid values: ws, grpc, kafka, rabbitmq.");
      }

      appService = std::make_shared<ApplicationService>(transport, std::chrono::seconds(period));
    }

  } // namespace service

} // namespace rating_calculator
