add_executable(ege_example
  native/main.cpp
)

target_link_libraries(ege_example PRIVATE ege_core)
if(TARGET ege_sdl)
  target_link_libraries(ege_example PRIVATE ege_sdl)
endif()

if(TARGET ege_sdl)
  add_executable(ege_sdl_example
    sdl/main.cpp
  )
  target_link_libraries(ege_sdl_example PRIVATE ege_sdl ege_core)
  # Ensure example target has SDL include dirs/linkage available for headers
  include(FindPkgConfig)
  find_package(SDL2 CONFIG QUIET)
  if (TARGET SDL2::SDL2)
    target_link_libraries(ege_sdl_example PRIVATE SDL2::SDL2)
  else()
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(SDL2_PKG QUIET sdl2)
      if (SDL2_PKG_FOUND)
        target_include_directories(ege_sdl_example PRIVATE ${SDL2_PKG_INCLUDE_DIRS})
        target_link_libraries(ege_sdl_example PRIVATE ${SDL2_PKG_LIBRARIES})
      endif()
    endif()
  endif()
endif()
