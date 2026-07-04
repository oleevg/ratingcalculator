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

    using UserIdentifier = std::uint64_t;

    class DealInformation;
    std::ostream& operator<<(std::ostream& out, const DealInformation& obj);

    class UserInformation;
    std::ostream& operator<<(std::ostream& out, const UserInformation& obj);

    /**
     * @brief User identification type wrapper used for serialization purposes.
     */
    struct UserIdInformation {
      explicit UserIdInformation(UserIdentifier id) : id(id)
      {}

      bool operator==(const UserIdInformation& obj) const
      {
        return (id == obj.id);
      }

      UserIdentifier id;
    };

    /**
     * @brief Describes user information.
     */
    struct UserInformation {
      UserInformation(const UserIdentifier& id, const std::string& name) : id(id), name(name)
      {}

      bool operator==(const UserInformation& obj) const
      {
        return ((id == obj.id) && (name == obj.name));
      }

      UserIdentifier id;
      std::string name;
    };

    /**
     * @brief Describes user deal information.
     */
    struct DealInformation {
      DealInformation(const UserIdentifier& userId, std::uint64_t timestamp, float amount)
          : userId(userId), timestamp(timestamp), amount(amount)
      {}

      bool operator==(const DealInformation& obj) const
      {
        return ((userId == obj.userId) && (timestamp == obj.timestamp) && (amount == obj.amount));
      }

      UserIdentifier userId;
      std::uint64_t timestamp;
      float amount;
    };

    /**
     * @brief Describes user rating position.
     */
    struct UserPosition {
      UserPosition(const UserInformation& userInformation, std::size_t position, float amount)
          : userInformation(userInformation), position(position), amount(amount)
      {}

      bool operator==(const UserPosition& obj) const
      {
        return ((userInformation == obj.userInformation) && (position == obj.position) && (amount == obj.amount));
      }

      UserInformation userInformation;
      std::size_t position;
      float amount;
    };

    using UserPositionsCollection = std::vector<UserPosition>;

    /**
     * @brief Describes user relative rating information.
     * @detailed Includes the specified user rating information as well as head, low and high users rating information
     * in relation to the requested user position.
     */
    struct UserRelativeRating {
      UserRelativeRating(const UserPosition& userPosition, const UserPositionsCollection& headPositions,
                         const UserPositionsCollection& highPositions, const UserPositionsCollection& lowPositions)
          : userPosition(userPosition), headPositions(headPositions), highPositions(highPositions),
            lowPositions(lowPositions)
      {}

      bool operator==(const UserRelativeRating& obj) const
      {
        return ((userPosition == obj.userPosition) && (headPositions == obj.headPositions) &&
                (highPositions == obj.highPositions) && (lowPositions == obj.lowPositions));
      }

      UserPosition userPosition;
      UserPositionsCollection headPositions;
      UserPositionsCollection highPositions;
      UserPositionsCollection lowPositions;
    };

  } // namespace core

} // namespace rating_calculator

#endif // RATINGCALCULATOR_TYPES_HPP
