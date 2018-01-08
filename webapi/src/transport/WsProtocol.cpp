/*
 * WsProtocol.cpp
 *
 *  Created on: 1/7/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <cstdint>

#include <webapi/transport/WsProtocol.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace transport {

      WsProtocol::WsProtocol(size_t resendNumber, int resendTimeout) :
              resendNumber_(resendNumber), resendTimeout_(resendTimeout), inCounter_(0), outCounter_(0)
      {}


      size_t WsProtocol::getInCounter()
      {
        return inCounter_;
      }

      size_t WsProtocol::getNextOutCounter()
      {
        return (outCounter_++)%SIZE_MAX;
      }


    }

  }

}