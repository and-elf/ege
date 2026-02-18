add_library(ege_core STATIC
  allocator.cpp
  # render pipeline is header-first for now; tests include headers directly
)

target_include_directories(ege_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)
target_compile_features(ege_core PUBLIC cxx_std_20)
