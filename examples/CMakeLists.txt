add_executable(Examples)
target_sources(
  Examples
  PRIVATE
  FILE_SET HEADERS
  BASE_DIRS
  "${CMAKE_SOURCE_DIR}/examples/"
  FILES "${CMAKE_SOURCE_DIR}/examples/lab/list/list.hpp"
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_compile_features(
  Examples
  PRIVATE
  cxx_std_23
)
target_link_libraries(
  Examples
  PRIVATE
  LabMemory::LabMemory
)
