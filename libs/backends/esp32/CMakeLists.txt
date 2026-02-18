cmake_minimum_required(VERSION 3.21)

set(ESP32_BACKEND_SOURCES
  src/esp32_backend.cpp
)

add_library(ege_esp32 STATIC ${ESP32_BACKEND_SOURCES})
target_include_directories(ege_esp32 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_features(ege_esp32 PUBLIC cxx_std_20)
target_link_libraries(ege_esp32 PUBLIC ege_core)

# ESP32 backend is a stub in this repo; real integration requires an ESP32 toolchain and drivers.
