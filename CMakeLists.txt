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
add_executable(server-tcp ${TEST_DIR}/server_tcp.cpp)
add_executable(client-tcp ${TEST_DIR}/client_tcp.cpp)
add_executable(server-udp ${TEST_DIR}/server_udp.cpp)
add_executable(client-udp ${TEST_DIR}/client_udp.cpp)

target_include_directories(server-tcp PUBLIC ${SOURCE_DIR})
target_include_directories(client-tcp PUBLIC ${SOURCE_DIR})
target_include_directories(server-udp PUBLIC ${SOURCE_DIR})
target_include_directories(client-udp PUBLIC ${SOURCE_DIR})

target_link_directories(server-tcp PUBLIC ${BUILD_DIR})
target_link_directories(client-tcp PUBLIC ${BUILD_DIR})
target_link_directories(server-udp PUBLIC ${BUILD_DIR})
target_link_directories(client-udp PUBLIC ${BUILD_DIR})

target_link_libraries(garnet ws2_32)
target_link_libraries(server-tcp garnet ws2_32)
target_link_libraries(client-tcp garnet ws2_32)
target_link_libraries(server-udp garnet ws2_32)
target_link_libraries(client-udp garnet ws2_32)
