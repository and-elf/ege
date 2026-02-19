# Header-only collision primitives
add_library(ege_physics_collision INTERFACE)
target_include_directories(ege_physics_collision INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(ege_physics_collision INTERFACE cxx_std_20)
