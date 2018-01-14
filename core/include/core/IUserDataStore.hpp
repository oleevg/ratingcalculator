/*
 * IUserDataStore.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_IUSERDATASTORE_HPP
#define RATINGCALCULATOR_IUSERDATASTORE_HPP

#include <string>

#include "Types.hpp"

namespace rating_calculator {

  namespace core {

    class IUserDataStore {
      public:
        virtual ~IUserDataStore() = default;

        virtual void addUser(const UserInformation& userInformation) = 0;

        virtual void renameUser(const UserIdentifier& userIdentifier, const std::string& name) = 0;

        virtual const UserInformation& getUserInformation(const UserIdentifier& userIdentifier) const = 0;
    };

  }

}

#endif //RATINGCALCULATOR_IUSERDATASTORE_HPP
