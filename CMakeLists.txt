cmake_minimum_required(VERSION 3.10)
project(Echo)

include(FetchContent)

set(CLAY_INCLUDE_ALL_EXAMPLES OFF)

FetchContent_Declare(
	raylib
	GIT_REPOSITORY https://github.com/raysan5/raylib
	GIT_TAG        5.5
)
FetchContent_Declare(
	clay
	GIT_REPOSITORY https://github.com/ketexon/clay
	GIT_TAG        main
)


FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(clay)

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
	clay
)

set_target_properties(
	Echo
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
		GENERATE_COMPILE_COMMANDS YES
)