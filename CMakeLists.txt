cmake_minimum_required(VERSION 3.30)

find_package(Git QUIET)
if(Git_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

  if(GIT_TAG)
    string(SUBSTRING ${GIT_TAG} 1 -1 PROJECT_VERSION)
  endif()
endif()

if(NOT PROJECT_VERSION)
  set(PROJECT_VERSION "1.0.0")
endif()

project(
  Allocators
  VERSION ${PROJECT_VERSION}
  LANGUAGES CXX
  DESCRIPTION "Laboratory work about allocators")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CI_CD_RELEASE)
  set(BUILD_SHARED_LIBS OFF)

  if(NOT APPLE)
    add_link_options(-static)
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|AppleClang")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
      add_compile_options(-mtune=generic -march=x86-64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
      add_compile_options(-mtune=generic -march=armv8-a)
    else()
      message(
        WARNING "[${PROJECT_NAME}] Unknown processor: ${CMAKE_SYSTEM_PROCESSOR}"
      )
    endif()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
      add_compile_options(/arch:AVX2)
    endif()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "Multithreaded")
  else()
    message(
      FATAL_ERROR
        "[${PROJECT_NAME}] Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
  endif()
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
endif()

if($ENV{DEV})
  include(cmake/ccache.cmake)
  include(cmake/clang_tidy.cmake)
endif()

find_package(Catch2 3 REQUIRED)
include(CTest)
include(Catch)

add_subdirectory(app)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
add_subdirectory(examples)
add_subdirectory(src)
