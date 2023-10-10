cmake_minimum_required(VERSION 3.16)
project(yoko)

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++11 -Wall -g -O0")

add_subdirectory(xml_parser)
add_subdirectory(smart_ptr)
add_subdirectory(connection_pool)