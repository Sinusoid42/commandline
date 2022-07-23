cmake_minimum_required(VERSION 2.8)
project( command_line_unit_test_01 )

include_directories("/usr/local/lib/")
set (CMAKE_CXX_STANDARD 11)
add_executable( command_line_unit_test_01 command_line_unit_test_01.cpp)

# add the MathFunctions library


