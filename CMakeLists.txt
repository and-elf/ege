cmake_minimum_required(VERSION 3.21)
project(ege LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common build options (inspired by cpp-best-practices/cmake_template)
option(EGE_WERROR "Treat warnings as errors" ON)
option(EGE_PEDANTIC_WARNINGS "Enable pedantic/extra warnings" ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  if(EGE_PEDANTIC_WARNINGS)
    add_compile_options(
      -Wall -Wextra -Wpedantic
      -Wconversion -Wsign-conversion -Wshadow
      -Wold-style-cast -Wnon-virtual-dtor -Wformat=2
    )
  endif()
  if(EGE_WERROR)
    add_compile_options(-Werror)
  endif()
elseif(MSVC)
  if(EGE_PEDANTIC_WARNINGS)
    add_compile_options(/W4 /permissive-)
  endif()
  if(EGE_WERROR)
    add_compile_options(/WX)
  endif()
endif()

option(EGE_BUILD_SDL "Build SDL backend and examples" ON)
option(EGE_BUILD_ESP32 "Build ESP32 backend (toolchain required)" OFF)
option(EGE_BUILD_TESTS "Build unit tests" ON)
option(EGE_COVERAGE "Enable code coverage instrumentation (for tests)" OFF)

add_subdirectory(src/engine)
add_subdirectory(libs/backends)
add_subdirectory(libs/physics)
add_subdirectory(examples)

if(EGE_BUILD_TESTS)
    enable_testing()
    if(EGE_COVERAGE)
        target_compile_options(ege_core PRIVATE --coverage)
        target_link_libraries(ege_core PRIVATE --coverage)
    endif()
  add_subdirectory(tests)
endif()
