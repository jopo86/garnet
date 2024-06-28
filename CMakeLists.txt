cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

file(GLOB SOURCE_FILES ${SOURCE_DIR}/*.cpp)

project(Garnet)

add_compile_options(-Wno-delete-incomplete)

add_library(garnet ${SOURCE_FILES})
add_executable(server ${TEST_DIR}/server.cpp)
add_executable(client ${TEST_DIR}/client.cpp)

target_include_directories(server PUBLIC ${SOURCE_DIR})
target_include_directories(client PUBLIC ${SOURCE_DIR})

target_link_directories(server PUBLIC ${BUILD_DIR})
target_link_directories(client PUBLIC ${BUILD_DIR})

target_link_libraries(garnet ws2_32)
target_link_libraries(server garnet ws2_32)
target_link_libraries(client garnet ws2_32)
