cmake_minimum_required(VERSION 3.8)
project("P5c IDE")

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(P5cIDE ${SOURCE_FILES})

find_package(wxWidgets COMPONENTS adv core base REQUIRED)
include("${wxWidgets_USE_FILE}")
target_link_libraries(P5cIDE ${wxWidgets_LIBRARIES})