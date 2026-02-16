add_executable(App "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_compile_features(
  App
  PRIVATE
  cxx_std_23
)
target_link_libraries(
  App
  PRIVATE
  LabMemory::LabMemory
)
set_target_properties(
  App
  PROPERTIES
  OUTPUT_NAME "app"
)
