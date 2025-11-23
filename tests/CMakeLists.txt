add_executable(AllocatorTests)
target_sources(
  AllocatorTests
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/tests.cpp"
)
target_compile_features(
  AllocatorTests
  PRIVATE
  cxx_std_23
)
target_link_libraries(
  AllocatorTests
  PRIVATE
  MemoryPoolModule::MemoryPoolModule
  Catch2::Catch2
  Catch2::Catch2WithMain
)
set_target_properties(
  AllocatorTests
  PROPERTIES
  OUTPUT_NAME "pool-tests"
)

catch_discover_tests(AllocatorTests)