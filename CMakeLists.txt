cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_EXAMPLES ON)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)

project(Garnet VERSION 1.0.0)

file(GLOB SOURCE_FILES ${SOURCE_DIR}/*.cpp)

add_library(garnet ${SOURCE_FILES})
target_compile_features(garnet PUBLIC cxx_std_20)

if(BUILD_EXAMPLES)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples)
endif()

if(WIN32)
    target_link_libraries(garnet ws2_32)
endif()
