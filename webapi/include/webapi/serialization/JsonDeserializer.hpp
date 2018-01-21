/*
 * JsonDeserializer.hpp
 *
 *  Created on: 1/5/18
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef RATINGCALCULATOR_JSONDESERIALIZER_HPP
#define RATINGCALCULATOR_JSONDESERIALIZER_HPP

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <core/EnumConverter.hpp>

#include "SerializerException.hpp"

namespace rating_calculator {

  namespace webapi {

    namespace serialization {

      template<typename T>
      class ViewTypeOnCompile;


      /**
       * @brief Custom type deserializer.
       * @tparam T Custom type to deserialize.
       * @tparam Enable
       */
      template<class T, class Enable = void>
      struct JsonDeserializer {
        static T Parse(const boost::property_tree::ptree& tree)
        {
          ViewTypeOnCompile<T> variableNameType;
          static_assert(sizeof(T) == -1, "You should specify this template to deserialize custom classes.");
        }
      };

      template<class Numeric>
      struct JsonDeserializer<Numeric, typename std::enable_if<std::is_integral<Numeric>::value>::type> {
        static Numeric Parse(const boost::property_tree::ptree& tree)
        {
          return tree.get_value<Numeric>();
        }
      };

      template<class Float>
      struct JsonDeserializer<Float, typename std::enable_if<std::is_floating_point<Float>::value>::type> {
        static Float Parse(const boost::property_tree::ptree& tree)
        {
          return tree.get_value<Float>();
        }
      };

      template<class Enum>
      struct JsonDeserializer<Enum, typename std::enable_if<std::is_enum<Enum>::value>::type> {
        static Enum Parse(const boost::property_tree::ptree& tree)
        {
          std::string strEnum = tree.get_value<std::string>();

          return core::EnumConverter<Enum>::get_const_instance().toEnum(strEnum);
        }
      };

      template<>
      struct JsonDeserializer<std::string> {
          static std::string Parse(const boost::property_tree::ptree& tree)
          {
            return tree.get_value<std::string>();
          }
      };


      template<class T>
      struct JsonDeserializer<std::vector<T>> {
          static std::vector<T> Parse(const boost::property_tree::ptree& value)
          {
            if (value.empty())
            {
              return std::vector<T>();
            }

            std::vector<T> result;
            result.reserve(value.size());
            for (const auto& item : value)
            {
              result.push_back(JsonDeserializer<T>::Parse(item.second));
            }

            return result;
          }
      };

    }
  }
}

#endif //RATINGCALCULATOR_JSONDESERIALIZER_HPP
