add_subdirectory(new_memory_resource)
add_subdirectory(null_memory_resource)

add_library(LabMemory INTERFACE)
add_library(LabMemory::LabMemory ALIAS LabMemory)
target_sources(
  LabMemory
  INTERFACE
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_SOURCE_DIR}/include/"
  FILES "${CMAKE_SOURCE_DIR}/include/lab/memory/memory.hpp"
)
target_link_libraries(
  LabMemory
  INTERFACE
  NullMemoryResource::NullMemoryResource
  NewMemoryResource::NewMemoryResource
)
