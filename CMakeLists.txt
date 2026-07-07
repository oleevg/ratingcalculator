##############################################
#
# CMake project build suite v1.1.0
# Author: Oleg F., fedorov.ftf@gmail.com
#
##############################################

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(ratingCalculator C CXX)

include(CMakeLists.var)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# ── clang-format / clang-tidy targets ──────────────────────────────────────

file(GLOB_RECURSE ALL_PROJECT_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/core/*.cpp"
  "${CMAKE_SOURCE_DIR}/core/*.hpp"
  "${CMAKE_SOURCE_DIR}/tempstore/*.cpp"
  "${CMAKE_SOURCE_DIR}/tempstore/*.hpp"
  "${CMAKE_SOURCE_DIR}/service/*.cpp"
  "${CMAKE_SOURCE_DIR}/service/*.hpp"
  "${CMAKE_SOURCE_DIR}/testClient/*.cpp"
  "${CMAKE_SOURCE_DIR}/testClient/*.hpp"
  "${CMAKE_SOURCE_DIR}/webapi/*.cpp"
  "${CMAKE_SOURCE_DIR}/grpcapi/*.hpp"
  "${CMAKE_SOURCE_DIR}/grpcapi/*.cpp"
)
list(FILTER ALL_PROJECT_SOURCES EXCLUDE REGEX ".*/websockets/.*")

find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-18 clang-format-17 clang-format-16)
if(CLANG_FORMAT_EXE)
  add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_PROJECT_SOURCES}
    COMMENT "Formatting sources with ${CLANG_FORMAT_EXE}"
    VERBATIM
  )
  add_custom_target(format-check
    COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${ALL_PROJECT_SOURCES}
    COMMENT "Checking source formatting with ${CLANG_FORMAT_EXE}"
    VERBATIM
  )
else()
  message(STATUS "clang-format not found — format/format-check targets disabled")
endif()

find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-18 clang-tidy-17 clang-tidy-16)
if(CLANG_TIDY_EXE)
  file(GLOB_RECURSE TIDY_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/tempstore/*.cpp"
    "${CMAKE_SOURCE_DIR}/service/*.cpp"
    "${CMAKE_SOURCE_DIR}/testClient/*.cpp"
    "${CMAKE_SOURCE_DIR}/webapi/src/*.cpp"
  )
  set(TIDY_EXTRA_ARGS "")
  if(APPLE)
    execute_process(
      COMMAND xcrun --show-sdk-path
      OUTPUT_VARIABLE MACOS_SDK_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    list(APPEND TIDY_EXTRA_ARGS --extra-arg=--sysroot=${MACOS_SDK_PATH})
  endif()
  add_custom_target(tidy
    COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${TIDY_EXTRA_ARGS} ${TIDY_SOURCE_FILES}
    COMMENT "Running clang-tidy"
    VERBATIM
  )
  add_custom_target(tidy-fix
    COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} --fix --fix-errors ${TIDY_EXTRA_ARGS} ${TIDY_SOURCE_FILES}
    COMMENT "Running clang-tidy with auto-fix"
    VERBATIM
  )
else()
  message(STATUS "clang-tidy not found — tidy/tidy-fix targets disabled")
endif()

# ── Smoke tests (integration) ───────────────────────────────────────────────

add_test(
  NAME smoke
  COMMAND ${CMAKE_SOURCE_DIR}/test/smoke.sh
    $<TARGET_FILE:ratingCalculator_service>
    $<TARGET_FILE:ratingCalculator_testClient>
)
set_tests_properties(smoke PROPERTIES
  TIMEOUT 60
  LABELS  integration
)