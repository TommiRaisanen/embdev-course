cmake_minimum_required(VERSION 3.24.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../test_program")

set (This TestSuite)

set(Sources
	TestSuite.cpp
)

include(CTest)

add_executable(${This} ${Sources})
target_include_directories(${This} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${This} PUBLIC
	gtest_main
	TimeParser
)

add_test(
	NAME ${This}
	COMMAND ${This}
)

