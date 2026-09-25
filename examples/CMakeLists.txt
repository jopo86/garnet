cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

file(GLOB SOURCE_FILES ${SOURCE_DIR}/*.cpp)

project(GarnetExamples)

add_executable(server_tcp ${SOURCE_DIR}/server_tcp.cpp)
add_executable(client_tcp ${SOURCE_DIR}/client_tcp.cpp)
add_executable(server_tcp_class ${SOURCE_DIR}/server_tcp_class.cpp)
add_executable(client_tcp_class ${SOURCE_DIR}/client_tcp_class.cpp)
add_executable(server_udp ${SOURCE_DIR}/server_udp.cpp)
add_executable(client_udp ${SOURCE_DIR}/client_udp.cpp)
add_executable(server_udp_class ${SOURCE_DIR}/server_udp_class.cpp)
add_executable(client_udp_class ${SOURCE_DIR}/client_udp_class.cpp)

if(WIN32)
    target_link_libraries(server_tcp        garnet ws2_32)
    target_link_libraries(client_tcp        garnet ws2_32)
    target_link_libraries(server_tcp_class  garnet ws2_32)
    target_link_libraries(client_tcp_class  garnet ws2_32)
    target_link_libraries(server_udp        garnet ws2_32)
    target_link_libraries(client_udp        garnet ws2_32)
    target_link_libraries(server_udp_class  garnet ws2_32)
    target_link_libraries(client_udp_class  garnet ws2_32)

else()
    target_link_libraries(server_tcp        garnet)
    target_link_libraries(client_tcp        garnet)
    target_link_libraries(server_tcp_class  garnet)
    target_link_libraries(client_tcp_class  garnet)
    target_link_libraries(server_udp        garnet)
    target_link_libraries(client_udp        garnet)
    target_link_libraries(server_udp_class  garnet)
    target_link_libraries(client_udp_class  garnet)

endif()
