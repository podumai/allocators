message(CHECK_START "Detecting clang-tidy support")

find_program(CLANG_TIDY_EXE NAMES clang-tidy)

if(CLANG_TIDY_EXE)
  message(CHECK_PASS "found")
  set(
    CMAKE_CXX_CLANG_TIDY
    "${CLANG_TIDY_EXE}"
    "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
  )
else()
  message(CHECK_FAIL "not found")
  message(WARNING "[${PROJECT_NAME}] clang-tidy disabled - install it for static analysis")
endif()
