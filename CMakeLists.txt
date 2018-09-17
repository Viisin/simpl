
option(simpl_build_tests "Build SIMPL's unit-test." ON)

include(cmake/common.cmake)
config_compiler_and_linker()

project(simpl CXX C)
cmake_minimum_required(VERSION 2.6.4)

set(simpl_include_dirs
	"${simpl_SOURCE_DIR}/include"
	"${simpl_SOURCE_DIR}")
include_directories(${simpl_include_dirs})

cxx_library(simpl "${cxx_strict}" src/simpl.c)
if (simpl_build_tests)
	cxx_executable(simpl-test-main unit-test simpl)
endif()
