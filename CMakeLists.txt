# This file is a part of  openglr
# Copyright (C) 2023  akshay bansod <akshayb@gmx.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)

project(openglr LANGUAGES C CXX)

# build dependencies
add_subdirectory(deps/glfw)
add_subdirectory(deps/glad)
add_subdirectory(deps/glad)
file(GLOB_RECURSE openglr_srcs src/**.cpp)
file(GLOB_RECURSE openglr_headers src/**.hpp)

message(STATUS "openglr sources ${openglr_srcs}")

add_library(openglr ${openglr_srcs})
target_link_libraries(openglr PUBLIC glfw glad imgui plog)
target_include_directories(openglr PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_precompile_headers(openglr PUBLIC ${CMAKE_SOURCE_DIR}/src/pch.hpp)


# Ctest integration
include(CTest)

link_libraries(openglr)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)


# generate a ctest for all cpp files in tests directory
file(GLOB test_srcs ${CMAKE_SOURCE_DIR}/tests/*.cpp)
foreach(test_src ${test_srcs})
    get_filename_component(test_src_name ${test_src} NAME_WLE)
    add_executable(${test_src_name} "${test_src}")
    add_test(${test_src_name} "${CMAKE_SOURCE_DIR}/bin/${test_src_name}")
endforeach(test_src test_srcs)
