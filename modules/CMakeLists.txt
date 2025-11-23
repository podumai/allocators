add_library(MemoryPoolModule)
add_library(MemoryPoolModule::MemoryPoolModule ALIAS MemoryPoolModule)
target_sources(
  MemoryPoolModule
  PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/"
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/memory_pool.cppm"
  PRIVATE
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_SOURCE_DIR}/include"
  FILES "${CMAKE_SOURCE_DIR}/include/tpu/modules/module_helper_macros.hpp"
)
target_compile_definitions(
  MemoryPoolModule
  PRIVATE
  MEMORY_POOL_TRACE
)
target_compile_features(
  MemoryPoolModule
  PRIVATE
  cxx_std_20
)

add_library(ForwardListModule)
add_library(ForwardListModule::ForwardListModule ALIAS ForwardListModule)
target_sources(
  ForwardListModule
  PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/lab_forward_list.cppm"
  PRIVATE
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_SOURCE_DIR}/include"
  FILES "${CMAKE_SOURCE_DIR}/include/tpu/helper_macros.hpp"
)
target_compile_features(
  ForwardListModule
  PRIVATE
  cxx_std_23
)