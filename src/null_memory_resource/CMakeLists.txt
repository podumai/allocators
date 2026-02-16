add_library(NullMemoryResource)
add_library(NullMemoryResource::NullMemoryResource ALIAS NullMemoryResource)
target_sources(
  NullMemoryResource
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_SOURCE_DIR}/include"
  FILES "${CMAKE_SOURCE_DIR}/include/lab/memory/details/null_memory_resource.hpp"
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/null_memory_resource.cpp"
)
set_target_properties(
  NullMemoryResource
  PROPERTIES
  OUTPUT_NAME "null_memory_resource"
)
