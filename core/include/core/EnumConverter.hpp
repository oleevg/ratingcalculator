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

        EnumConverter();

        void addTranslation(const T& key, const std::string& value)
        {
          enumToString.insert({key, value});
          stringToEnum.insert({value, key});
        }
        const std::string& toString(const T& key) const
        {
          return enumToString.at(key);
        }

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
