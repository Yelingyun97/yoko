cmake_minimum_required(VERSION 2.8)
project(xml-parser)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++11 -Wall -g -O0")

set(LIB_SRC 
    xml-parser/Node.cpp
    xml-parser/Xml.cpp
)

add_library(yoko SHARED ${LIB_SRC})
add_executable(test tests/test.cpp)
add_dependencies(test yoko)
target_link_libraries(test yoko)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)