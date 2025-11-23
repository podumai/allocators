cmake_minimum_required(VERSION 3.30)
project(
  Allocators
  VERSION 1.0.0
  LANGUAGES CXX
  DESCRIPTION "Laboratory work about allocators"
)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

if($ENV{DEV})
  include(cmake/ccache.cmake)
  include(cmake/clang_tidy.cmake)
endif()

find_package(Catch2 3 REQUIRED)
include(CTest)
include(Catch)

add_subdirectory(modules)
add_subdirectory(app)
add_subdirectory(tests)
add_subdirectory(examples)
