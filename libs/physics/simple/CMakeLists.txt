# Header-only simple physics system
add_library(ege_physics_simple INTERFACE)
target_include_directories(ege_physics_simple INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(ege_physics_simple INTERFACE ege_physics_collision)
target_compile_features(ege_physics_simple INTERFACE cxx_std_20)
