
cmake_minimum_required(VERSION 3.21)

include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

add_executable(ege_unit_tests
	allocator_test.cpp
	render_pipeline_test.cpp
	command_buffer_test.cpp
)

target_link_libraries(ege_unit_tests PRIVATE ege_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(ege_unit_tests)
