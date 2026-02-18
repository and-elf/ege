cmake_minimum_required(VERSION 3.21)
project(ege LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EGE_BUILD_SDL "Build SDL backend and examples" ON)
option(EGE_BUILD_ESP32 "Build ESP32 backend (toolchain required)" OFF)
option(EGE_BUILD_TESTS "Build unit tests" ON)
option(EGE_COVERAGE "Enable code coverage instrumentation (for tests)" OFF)

add_subdirectory(src/engine)
add_subdirectory(libs/backends)
add_subdirectory(examples)

if(EGE_BUILD_TESTS)
    enable_testing()
    if(EGE_COVERAGE)
        target_compile_options(ege_core PRIVATE --coverage)
        target_link_libraries(ege_core PRIVATE --coverage)
    endif()
  add_subdirectory(tests)
endif()
