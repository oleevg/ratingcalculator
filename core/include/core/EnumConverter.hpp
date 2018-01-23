/*
 * EnumConverter.hpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_ENUMCONVERTER_HPP
#define RATINGCALCULATOR_ENUMCONVERTER_HPP

#include <map>
#include <string>

#include <boost/serialization/singleton.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/range/algorithm/lexicographical_compare.hpp>

namespace rating_calculator {

  namespace core {

    /**
     * @brief Class responsible for Enum to string conversion.
     * @tparam T Specific Enum type to specify conversions for.
     */
    template<class T>
    class EnumConverter : public boost::serialization::singleton<EnumConverter<T> > {
        struct StringInCaseLess {
          bool operator()(const std::string& lhs, const std::string& rhs) const
          {
            return boost::range::lexicographical_compare(lhs, rhs, boost::is_iless());
          }
        };

      public:
        typedef std::map<T, std::string> EnumToString;
        typedef std::map<std::string, T, StringInCaseLess> StringToEnum;

        /**
         * @brief ctor.
         */
        EnumConverter();

        /**
         * @brief Adds new enum to string conversion.
         * @param key Enum value to specify conversion for.
         * @param value String representation.
         */
        void addConversion(const T& key, const std::string& value)
        {
          enumToString.insert({key, value});
          stringToEnum.insert({value, key});
        }

        /**
         * @brief Provides string representation for enum value.
         * @param key Enum value to get conversion for.
         * @return Enum value string representation.
         */
        const std::string& toString(const T& key) const
        {
          return enumToString.at(key);
        }

        /**
         * @brief Provides enum type corresponding to the string representation.
         * @param key String representation.
         * @return Enum value corresponding to the string representation.
         */
        T toEnum(const std::string& key) const
        {
          return stringToEnum.at(key);
        }

      private:
        EnumToString enumToString;
        StringToEnum stringToEnum;

    };

  }

}


#endif //RATINGCALCULATOR_ENUMCONVERTER_HPP
