cmake_minimum_required(VERSION 3.10)
project(Echo)

include(FetchContent)

FetchContent_Declare(
	raylib
	GIT_REPOSITORY https://github.com/raysan5/raylib
	GIT_TAG        5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(
	Echo
	src/main.cpp
)

target_include_directories(
	Echo
	PRIVATE
	${CMAKE_SOURCE_DIR}/ext/layout/
)

target_link_libraries(
	Echo
	raylib
)

set_target_properties(
	Echo
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
		GENERATE_COMPILE_COMMANDS YES
)