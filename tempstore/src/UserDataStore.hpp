/*
 * UserDataStore.hpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_USERDATASTORE_HPP
#define RATINGCALCULATOR_USERDATASTORE_HPP

#include <unordered_map>

#include <core/IUserDataStore.hpp>

namespace rating_calculator {

  namespace tempstore {

    class UserDataStore : public core::IUserDataStore {
      public:
        typedef std::unordered_map<core::UserIdentifier, core::UserInformation> UsersCollection;

      public:

        void addUser(const core::UserInformation& userInformation) override;

        void renameUser(const core::UserIdentifier& userIdentifier, const std::string& name) override;

        const core::UserInformation& getUserInformation(const core::UserIdentifier& userIdentifier) const override;

      private:
        UsersCollection users;
    };
  }

}


#endif //RATINGCALCULATOR_USERDATASTORE_HPP
