/*
 * GrowingKeyAssocVector.hpp
 *
 *  Created on: 1/16/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_GROWINGKEYASSOCVECTOR_HPP
#define RATINGCALCULATOR_GROWINGKEYASSOCVECTOR_HPP

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <memory>
#include <sstream>

#include "BaseException.hpp"

namespace rating_calculator {

  namespace core {

    /**
     * @brief Multiple keys container optimized for updates for the second key.
     * @tparam AccessKey The first key type with complexity O(1) for access operations.
     * @tparam SortKey The second key type with complexity ~ O(ln(N)) for access operations.
     * @tparam Value Custom type to be stored in container.
     * @tparam accessKey Custom type public field pointer used as access key.
     * @tparam sortKey Custom type public field pointer used as sort key.
     * @tparam Modify A binary function object type describing data modification during update operations.
     * @tparam Hash A unary function object type describing hashing.
     * @tparam Pred A binary predicate describing items equality.
     * @tparam Comp A binary predicate describing sorted items ordering.
     */
    template <class AccessKey, class SortKey, class Value, AccessKey Value::*accessKey, SortKey Value::*sortKey, class Modify = std::plus<SortKey>, class Hash = std::hash<AccessKey>, class Pred = std::equal_to<AccessKey>, class Comp = std::greater<SortKey>>
    class MultiKeyVolatileContainer {
        typedef std::shared_ptr<Value> ValuePtr;

      public:
        typedef std::vector<ValuePtr> SortContainer;
        typedef std::unordered_map<AccessKey, ValuePtr, Hash, Pred> AccessContainer;

      public:
        struct PositionedData {
          PositionedData(size_t _position, const Value& value_):
          position(_position), value(value_)
          {

          }

          size_t position;
          Value value;
        };

        typedef std::vector<PositionedData> PositionedDataContainer;

        struct CompPtr {
          bool operator()(const ValuePtr& lhs, const ValuePtr& rhs)
          {
            return Comp()(*lhs.*sortKey, *rhs.*sortKey);
          }
        };

      public:
        MultiKeyVolatileContainer(size_t initialSize)
        {
          sortData_.reserve(initialSize);
          accessData_.reserve(initialSize);
        }

        void clear()
        {
          sortData_.clear();
          accessData_.clear();
        }

        void insert(const Value& value)
        {
          if(accessData_.count(value.*accessKey))
          {
            // Update existing item
            auto sortIter = getSortNode(value);

            auto oldValue = (**sortIter).*sortKey;
            auto newValue = Modify()((**sortIter).*sortKey, value.*sortKey);
            (**sortIter).*sortKey = newValue;

            std::cout << "User: " << value.*accessKey << ", oldValue: " << oldValue << ", newValue: " << newValue << std::endl;

            if(Comp()(newValue, oldValue))
            {
              // Have to move left
              auto shiftIter = sortIter ;
              while(shiftIter != sortData_.begin())
              {
                shiftIter--;
                if(CompPtr()(*shiftIter, *sortIter))
                {
                  break;
                }
                else
                {
                  std::swap(*shiftIter, *sortIter);
                }

                sortIter = shiftIter;
              }
            }
            else
            {
              // Have to move right
              auto shiftIter = sortIter ;
              while(shiftIter != sortData_.end() - 1)
              {
                shiftIter++;
                if(CompPtr()(*shiftIter, *sortIter))
                {
                  std::swap(*shiftIter, *sortIter);
                }
                else
                {
                  break;
                }

                sortIter = shiftIter;
              }
            }

          }
          else
          {
            // insert new item
            auto valuePtr = std::make_shared<Value>(value);
            auto iter = std::lower_bound(sortData_.begin(), sortData_.end(), valuePtr, CompPtr());
            sortData_.insert(iter, valuePtr);

            accessData_.insert({value.*accessKey, valuePtr});
          }
        }

        PositionedData findWithPosition(const AccessKey& key) const
        {
          auto sortIter = findInSortContainer(key);

          return PositionedData(std::distance(sortData_.begin(), sortIter), **sortIter);
        }

        PositionedDataContainer getHeadPositions(size_t nPositions) const
        {
          PositionedDataContainer result;
          result.reserve(nPositions);

          for (size_t i = 0; i < nPositions && i < sortData_.size(); ++i)
          {
            result.emplace_back(i, *sortData_[i]);
          }

          return result;
        }

        PositionedDataContainer getHighPositions(const AccessKey& key, size_t nPositions) const
        {
          PositionedDataContainer result;
          result.reserve(nPositions);

          auto sortIter = findInSortContainer(key);
          size_t position = std::distance(sortData_.begin(), sortIter);

          auto beginIter = sortData_.begin();
          if (position >= nPositions)
          {
            beginIter = sortIter - nPositions;
          }

          for (auto iter = beginIter; iter != sortIter; ++iter)
          {
            result.emplace_back(std::distance(sortData_.begin(), iter), **iter);
          }

          return result;
        }

        PositionedDataContainer getLowPositions(const AccessKey& key, size_t nPositions) const
        {
          PositionedDataContainer result;
          result.reserve(nPositions);

          auto sortIter = findInSortContainer(key);
          size_t positionFromEnd = std::distance(sortIter, sortData_.end() - 1);

          auto endIter = sortData_.end() - 1;
          if (positionFromEnd >= nPositions)
          {
            endIter = sortIter + 1 + nPositions;
          }

          for (auto iter = sortIter + 1; iter != endIter && iter != sortData_.end(); ++iter)
          {
            result.emplace_back(std::distance(sortData_.begin(), iter), **iter);
          }

          return result;
        }

        bool isPresent(const AccessKey& key) const
        {
          return accessData_.count(key);
        }

      private:
        typename SortContainer::iterator getSortNode(const Value& value) const
        {
          auto accessIter = accessData_.find(value.*accessKey);
          if(accessIter == accessData_.end())
          {
            std::stringstream strStream;
            strStream << "Item with access key '" << value.*accessKey << "' is not present in access container.";

            throw core::BaseException(strStream.str());
          }

          auto sortIter = findInSortContainer(accessIter->second);

        }

        typename SortContainer::iterator findInSortContainer(const ValuePtr& valuePtr)
        {
          auto result = static_cast<const MultiKeyVolatileContainer*>(this)->findInSortContainer(valuePtr);
          size_t dist = std::distance(sortData_.begin(), result);

          return (sortData_.begin() + dist);
        }

        typename SortContainer::const_iterator findInSortContainer(const ValuePtr& valuePtr) const
        {
          auto iter = std::lower_bound(sortData_.begin(), sortData_.end(), valuePtr, CompPtr());
          while ((**iter).*accessKey != *valuePtr.*accessKey)
          {
            if(iter == sortData_.end())
            {
              std::stringstream strStream;
              strStream << "Item with access key '" << *valuePtr.*accessKey << "' is not present in sort container.";

              throw core::BaseException(strStream.str());
            }

            ++iter;
          }

          return iter;
        }

        typename SortContainer::const_iterator findInSortContainer(const AccessKey& key) const
        {
          auto accessIter = accessData_.find(key);

          if(accessIter == accessData_.end())
          {
            std::stringstream strStream;
            strStream << "findInSortContainer() Item with access key '" << key << "' is not present in access container.";

            throw core::BaseException(strStream.str());
          }

          return findInSortContainer(accessIter->second);
        }

      private:
        SortContainer sortData_;
        AccessContainer accessData_;
    };

  }
}


#endif //RATINGCALCULATOR_GROWINGKEYASSOCVECTOR_HPP
