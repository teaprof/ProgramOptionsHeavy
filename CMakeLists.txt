cmake_minimum_required(VERSION 3.30)

project(ProgramOptions LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost 1.87.0 NO_MODULE REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
enable_testing()

include_directories(include)

add_subdirectory(examples)
add_subdirectory(src)
add_subdirectory(tests)
