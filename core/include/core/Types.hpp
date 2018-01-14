/*
 * Types.hpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_TYPES_HPP
#define RATINGCALCULATOR_TYPES_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

namespace rating_calculator {

  namespace core {

    typedef uint64_t UserIdentifier;

    class DealInformation;
    std::ostream& operator<<(std::ostream& out, const DealInformation& obj);

    class UserInformation;
    std::ostream& operator<<(std::ostream& out, const UserInformation& obj);

    struct UserIdInformation {
      explicit UserIdInformation(UserIdentifier id_) : id(id_)
      {}

      bool operator==(const UserIdInformation& obj) const
      {
        return (id == obj.id);
      }

      UserIdentifier id;
    };

    struct UserInformation {
      UserInformation(const UserIdentifier& id_, const std::string& name_) :
              id(id_), name(name_)
      {}

      bool operator==(const UserInformation& obj) const
      {
        std::cout << *this;
        std::cout << obj;

        return ((id == obj.id) && (name == obj.name));
      }

      UserIdentifier id;
      std::string name;
    };

    struct DealInformation {
      DealInformation(const UserIdentifier& userId_, uint64_t timestamp_, float amount_) :
              userId(userId_), timestamp(timestamp_), amount(amount_)
      {}

      bool operator==(const DealInformation& obj) const
      {
        std::cout << *this;
        std::cout << obj;

        return ((userId == obj.userId) && (timestamp == obj.timestamp) && (amount == obj.amount));
      }

      UserIdentifier userId;
      uint64_t timestamp;
      float amount;
    };

    struct UserPosition {
      UserPosition(const UserInformation& userInformation_, size_t position_, float amount_):
      userInformation(userInformation_), position(position_), amount(amount_)
      {}

      bool operator==(const UserPosition& obj) const
      {
        return ((userInformation == obj.userInformation) && (position == obj.position) && (amount == obj.amount));
      }

      UserInformation userInformation;
      size_t position;
      float amount;
    };


    typedef std::vector<UserPosition> UserPositionsCollection;

    struct UserRelativeRating {
      UserRelativeRating(const UserPosition& userPosition_, const UserPositionsCollection& headPositions_,
                         const UserPositionsCollection& highPositions_, const UserPositionsCollection& lowPositions_)
              : userPosition(userPosition_), headPositions(headPositions_), highPositions(highPositions_),
                lowPositions(lowPositions_)
      {}

      bool operator==(const UserRelativeRating& obj) const
      {
        return ((userPosition == obj.userPosition) && (headPositions == obj.headPositions) && (highPositions == obj.highPositions) && (lowPositions == obj.lowPositions));
      }

      UserPosition userPosition;
      UserPositionsCollection headPositions;
      UserPositionsCollection highPositions;
      UserPositionsCollection lowPositions;
    };

    /// TMP!
    inline std::ostream& operator<<(std::ostream& out, const DealInformation& obj)
    {
//      out << "--- User id: " << obj.userId << std::endl;
//      out << "--- Timestamp: " << obj.timestamp << std::endl;
//      out << "--- Amount: " << obj.amount << std::endl;

      return out;
    }

    inline std::ostream& operator<<(std::ostream& out, const UserInformation& obj)
    {
//      out << "--- User id: " << obj.id << std::endl;
//      out << "--- Name: " << obj.name << std::endl;

      return out;
    }

  }

}

#endif //RATINGCALCULATOR_TYPES_HPP
