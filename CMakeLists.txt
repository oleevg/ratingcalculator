##############################################
#
# CMake project build suite v1.1.0
# Author: Oleg F., fedorov.ftf@gmail.com
#
##############################################

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(ratingCalculator C CXX)

include(CMakeLists.var)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

set(BUILD_SHARED_LIBS OFF)

find_package(Threads REQUIRED)

enable_testing()

add_definitions(${${project_name}_definitions})

foreach (module ${modules})
  add_subdirectory(${module})
endforeach ()