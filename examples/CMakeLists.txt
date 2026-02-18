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
endif()
