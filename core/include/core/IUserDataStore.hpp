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

    /**
     * @brief Interface describing common users data store functionality.
     */
    class IUserDataStore {
      public:
        virtual ~IUserDataStore() = default;

        /**
         * @brief Adds new user to the store.
         * @param userInformation Information for user to be added.
         */
        virtual void addUser(const UserInformation& userInformation) = 0;

        /**
         * @brief Renames existent user.
         * @param userIdentifier User identifier to rename.
         * @param name New user's name.
         */
        virtual void renameUser(const UserIdentifier& userIdentifier, const std::string& name) = 0;

        /**
         * @brief Provides access to existent user information.
         * @param userIdentifier User identifier to get information for.
         * @return User information.
         */
        virtual const UserInformation& getUserInformation(const UserIdentifier& userIdentifier) const = 0;
    };

  }

}

#endif //RATINGCALCULATOR_IUSERDATASTORE_HPP
