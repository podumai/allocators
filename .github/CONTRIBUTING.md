# Contributing to [allocators]

Thank you for your interest in contributing to **allocators**! We welcome contributions from the community to help improve the project. Here are some guidelines to help you get started:

## Commit message Guidelines

We use [<ins>Conventional Commits</ins>](https://www.conventionalcommits.org/en/v1.0.0/). All PRs must follow this format:

`<type>[optional-scope]: <description>`

- **feat**: A new feature (e.g. `feat: add support for custom memory resources`)
- **fix**: A bug fix (e.g. `fix: correct memory leak in pool allocator`)
- **perf**: A code change that improves performance (e.g. `perf: optimize allocation algorithm for better performace`)
- **refactor**: A code change that neither fixes a bug nor adds a feature (e.g. `refactor: simplify memory resources interface`)
- **ci**: Changes to our CI/CD configuration or scripts (e.g. `ci: update Github Actions workflow for better testing`)
- **docs**: Documentation only changes (e.g. `docs: update README with new allocator examples`)
- **test**: Adding missing tests or correcting existing tests (e.g. `test: add unit tests for pool allocator`)
- **chore**: Other changes that don't modify src or test files (e.g. `chore: update dependencies`)

> **Note:** Commits that do not follow this format will be rejected by the CI/CD linter.

## Technical Standards

- **Formatting:** We use `clang-format` for C++ code and `cmake-format` for CMake files. Please ensure your code is properly formatted before submitting a PR.
- **Linting:** We use `clang-tidy` for C++ code and `cmakelint` for CMake files. Please ensure your code passes all linting checks before submitting a PR.
- **Error Handling:** All functions should handle errors gracefully and return appropriate `std::expected`, `std::optional` or other error handling types instead of throwing exceptions. Exception can be thrown only in cases where the error is unrecoverable and cannot be handler by the caller (e.g. out of memory).
- **Testing:** All new feature and bug fixes must include unit tests. We use `Catch2` as our testing framework. Please ensure your tests are comprehensive and cover all relevant edge cases.
- **Dependency Management:** We use `conan2` for dependency management. Please ensure that any new dependencies are added to the `conanfile.py` and that the project builds successfully with the new dependencies.
- **Build System:** We use `CMake` as our build system. Please ensure that any new source files are added to the appropriate `CMakeLists.txt` and that the project builds successfully with the new changes.
- **Code Documentation:** We use `Doxygen` for code documentation. Please ensure that all new code is properly documented with comments and that the documentation is up to date.
