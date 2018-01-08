/*
 * Application.cpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <string>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <core/ulog.h>
#include <core/SystemException.hpp>
#include <core/ThreadHelper.hpp>

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
      }
      catch (const core::SystemException& exc)
      {
        mdebug_error(exc.what());
        rc = exc.getErrorCode();
      }
      catch (const core::BaseException& exc)
      {
        mdebug_error(exc.what());
        rc = -1;
      }
      catch(const std::exception& exc)
      {
        mdebug_error("Unknown error occurred. %s", exc.what());
        rc = -1;
      }

      return rc;
    }

    void Application::parseArguments(int argc, const char** argv)
    {
      namespace options = boost::program_options;

      /* Default parameters definition */
      const int portDefault = 88888;
      const int periodDefault = 5;
      const size_t threadPoolSizeDefault = 5;
      const bool debugRunDefault = false;

      options::options_description optionDescription((boost::format("Usage: %s [options]... \nOptions") % argv[0]).str());

      int port = portDefault;
      int period = periodDefault;
      size_t threadPoolSize = threadPoolSizeDefault;
      bool debugRun = debugRunDefault;

      optionDescription.add_options()
              ("port,p", options::value<int>(&port)->default_value(portDefault), "The port number to listen.")
              ("period,t", options::value<int>(&period)->default_value(periodDefault), "Rating update period in seconds.")
              ("threads", options::value<size_t>(&threadPoolSize)->default_value(threadPoolSizeDefault), "The service's thread pool size.")
              ("debug,d", options::bool_switch(&debugRun)->default_value(debugRunDefault), "Interactive run without daemonizing.")
              ("help,h", "As it says.");

      options::variables_map variableMap;

      options::store(options::parse_command_line(argc, argv, optionDescription), variableMap);
      options::notify(variableMap);

      if(variableMap.count("help"))
      {
        std::cout << optionDescription << "\n";
        exit(0);
      }

      if(period <= 0)
      {
        throw core::BaseException("Negative or zero timeout values are not supported. Please provide some positive integer value.");
      }

      if(threadPoolSize < 1)
      {
        throw core::BaseException("The number of service's threads can't be less than one. Please provide some positive integer value.");
      }

      appService = std::make_shared<ApplicationService>(port, period, threadPoolSize);
    }
  }

}
