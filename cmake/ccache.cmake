message(CHECK_START "[${PROJECT_NAME}] Detecting ccache support")

find_program(CCACHE_EXE NAMES ccache)

if(CCACHE_EXE)
  message(CHECK_PASS "found")
  set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXE}")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXE}")
else()
  message(CHECK_FAIL "not found")
  message(WARNING "[${PROJECT_NAME}] Faster rebuilds disabled")
endif()
