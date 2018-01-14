/*
 * UserDataStore.cpp
 *
 *  Created on: 1/14/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#include <boost/format.hpp>

#include <core/BaseException.hpp>

#include "UserDataStore.hpp"

namespace rating_calculator {

  namespace tempstore {

    void UserDataStore::addUser(const core::UserInformation& userInformation)
    {
      if (users.count(userInformation.id))
      {
        throw core::BaseException((boost::format("Can't add user with identifier '%d'. User with such identifier have already been registered.") % userInformation.id).str());
      }

      users.insert({userInformation.id, userInformation});
    }

    void UserDataStore::renameUser(const core::UserIdentifier& userIdentifier, const std::string& name)
    {
      auto item = users.find(userIdentifier);

      if(item == users.end())
      {
        throw core::BaseException((boost::format("Can't rename user with identifier '%d'. User with such identifier is not registered.") % userIdentifier).str());
      }

      item->second.name = name;
    }

    const core::UserInformation& UserDataStore::getUserInformation(const core::UserIdentifier& userIdentifier) const
    {
      auto item = users.find(userIdentifier);

      if(item == users.end())
      {
        throw core::BaseException((boost::format("Can't provide information for user '%d'. User with such identifier is not registered.") % userIdentifier).str());
      }

      return item->second;
    }
  }

  }