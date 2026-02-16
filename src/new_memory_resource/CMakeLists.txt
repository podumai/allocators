add_library(NewMemoryResource)
add_library(NewMemoryResource::NewMemoryResource ALIAS NewMemoryResource)
target_sources(
  NewMemoryResource
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_SOURCE_DIR}/include"
  FILES "${CMAKE_SOURCE_DIR}/include/lab/memory/details/new_memory_resource.hpp"
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/new_memory_resource.cpp"
)
set_target_properties(
  NewMemoryResource
  PROPERTIES
  OUTPUT_NAME "new_memory_resource"
)
