/*
 * JsonSerializer.hpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_JSONSERIALIZER_HPP
#define RATINGCALCULATOR_JSONSERIALIZER_HPP

#include <vector>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <core/EnumConverter.hpp>

namespace rating_calculator {

  namespace webapi {

    namespace serialization {

      template<typename T>
      class ViewTypeOnCompile;

      template<class T, class Enable = void>
      struct JsonSerializer {
        static boost::property_tree::ptree Serialize(const T& value)
        {
          ViewTypeOnCompile <T> variableNameType;
          static_assert(sizeof(T) == -1, "You should specify this template to serialize custom classes.");
        }
      };

      template<class Numeric>
      struct JsonSerializer<Numeric, typename std::enable_if<std::is_integral<Numeric>::value>::type> {
        static boost::property_tree::ptree Serialize(Numeric value)
        {
          boost::property_tree::ptree result;
          result.put<Numeric>("", value);

          return result;
        }
      };

      template<class Float>
      struct JsonSerializer<Float, typename std::enable_if<std::is_floating_point<Float>::value>::type> {
        static boost::property_tree::ptree Serialize(Float value)
        {
          boost::property_tree::ptree result;
          result.put<Float>("", value);

          return result;
        }
      };

      template<class Enum>
      struct JsonSerializer< Enum, typename std::enable_if<std::is_enum<Enum>::value>::type> {
        static boost::property_tree::ptree Serialize(Enum value)
        {
          boost::property_tree::ptree result;
          result.put_value(core::EnumConverter<Enum>::get_const_instance().toString(value));

          return result;
        }
      };

      template<>
      struct JsonSerializer<std::string> {
        static boost::property_tree::ptree Serialize(const std::string& value)
        {
          boost::property_tree::ptree result;
          result.put_value(value);

          return result;
        }
      };

      template<class T>
      struct JsonSerializer<std::vector<T>> {
        static boost::property_tree::ptree Serialize(const std::vector<T>& value)
        {
          boost::property_tree::ptree result;

          for (const auto& item : value)
          {
            result.push_back({"", JsonSerializer<T>::Serialize(item)});
          }

          return result;
        }
      };


    }
  }
}

#endif //RATINGCALCULATOR_JSONSERIALIZER_HPP
