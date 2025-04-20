cmake_minimum_required(VERSION 3.10)
project(Echo LANGUAGES C CXX)

include(FetchContent)

set(CLAY_INCLUDE_ALL_EXAMPLES OFF)

FetchContent_Declare(
	raylib
	URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.zip
)

FetchContent_MakeAvailable(raylib)

add_executable(
	Echo
	src/main.cpp
)

target_include_directories(
	Echo
	PRIVATE
)

target_link_libraries(
	Echo
	raylib
)

set_target_properties(
	Echo
	PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
		GENERATE_COMPILE_COMMANDS YES
)