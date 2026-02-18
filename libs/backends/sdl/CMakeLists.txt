cmake_minimum_required(VERSION 3.21)

set(SDL_BACKEND_SOURCES
  src/sdl_backend.cpp
)

add_library(ege_sdl STATIC ${SDL_BACKEND_SOURCES})
target_include_directories(ege_sdl PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_features(ege_sdl PUBLIC cxx_std_20)
target_link_libraries(ege_sdl PUBLIC ege_core)

include(FindPkgConfig)

# Prefer system SDL via CMake config or pkg-config.
set(SDL_FOUND_SYSTEM OFF)
find_package(SDL2 CONFIG QUIET)
if (TARGET SDL2::SDL2)
  set(SDL_FOUND_SYSTEM ON)
  target_link_libraries(ege_sdl PRIVATE SDL2::SDL2)
endif()
if (NOT SDL_FOUND_SYSTEM AND PKG_CONFIG_FOUND)
  pkg_check_modules(SDL2_PKG QUIET sdl2)
  if (SDL2_PKG_FOUND)
    set(SDL_FOUND_SYSTEM ON)
    target_include_directories(ege_sdl PRIVATE ${SDL2_PKG_INCLUDE_DIRS})
    target_link_libraries(ege_sdl PRIVATE ${SDL2_PKG_LIBRARIES})
  endif()
endif()

if (NOT SDL_FOUND_SYSTEM)
  if (EGE_BUILD_SDL)
    message(FATAL_ERROR "SDL backend requested (EGE_BUILD_SDL=ON) but no system SDL found. Install libsdl2-dev or disable EGE_BUILD_SDL.")
  else()
    # build even without SDL (not recommended) — library will be compiled but will fail at link if SDL calls used
  endif()
endif()
