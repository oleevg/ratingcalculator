/*
 * main.cpp
 *
 *  Created on: 1/4/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <core/ulog.h>

#include "Application.hpp"

namespace rating_calculator {

  class ULogWrapper {
    public:
      ULogWrapper()
      {
        ulog_init();
      }

      ~ULogWrapper()
      {
        ulog_clean();
      }
  };

}

int main(int argc, const char* argv[])
{
  rating_calculator::ULogWrapper ulog;

  rating_calculator::service::Application app;

  return app.run(argc, argv);
}
