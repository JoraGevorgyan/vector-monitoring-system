cmake_minimum_required(VERSION 3.21)
project(vector_monitoring VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build the test targets" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)
option(ENABLE_CLANG_FORMAT_CHECK "Check formatting during the build" OFF)
option(ENABLE_COVERAGE "Instrument the code for coverage (gcov)" OFF)

file(GLOB_RECURSE FORMAT_FILES CONFIGURE_DEPENDS
	${CMAKE_SOURCE_DIR}/src/*.[ch]pp
	${CMAKE_SOURCE_DIR}/include/*.[ch]pp
	${CMAKE_SOURCE_DIR}/tests/*.[ch]pp
)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
	add_custom_target(apply_format
		COMMAND ${CLANG_FORMAT_EXECUTABLE} -i --style=file ${FORMAT_FILES}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Applying clang-format"
		VERBATIM
	)

	if(ENABLE_CLANG_FORMAT_CHECK)
		add_custom_target(format_check
			COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror --style=file ${FORMAT_FILES}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			COMMENT "Checking clang-format"
			VERBATIM
		)
	endif()
elseif(ENABLE_CLANG_FORMAT_CHECK)
	message(FATAL_ERROR "clang-format is required when ENABLE_CLANG_FORMAT_CHECK is enabled")
endif()

if(ENABLE_CLANG_TIDY)
	find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
	if(NOT CLANG_TIDY_EXECUTABLE)
		message(FATAL_ERROR "clang-tidy is required when ENABLE_CLANG_TIDY is enabled")
	endif()
	set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE})
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
	${CMAKE_SOURCE_DIR}/src/*/src/*.cpp
)

find_package(Threads REQUIRED)

add_library(vm_lib STATIC ${APP_SOURCES})
target_include_directories(vm_lib PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(vm_lib PUBLIC Threads::Threads)

target_compile_features(vm_lib PUBLIC cxx_std_17)

if(ENABLE_COVERAGE)
	if(MSVC)
		message(FATAL_ERROR "ENABLE_COVERAGE is not supported with MSVC")
	endif()
	target_compile_options(vm_lib PUBLIC --coverage -O0 -g)
	target_link_options(vm_lib PUBLIC --coverage)
endif()

if(MSVC)
	target_compile_options(vm_lib PRIVATE /W4 /permissive-)
else()
	target_compile_options(vm_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(vector_monitoring src/main.cpp)
target_link_libraries(vector_monitoring PRIVATE vm_lib)
target_compile_features(vector_monitoring PRIVATE cxx_std_17)

if(TARGET format_check)
	add_dependencies(vector_monitoring format_check)
endif()

include(CTest)
enable_testing()

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()
