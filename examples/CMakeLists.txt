cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(GNET_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(GNET_BUILD_DIR ${CMAKE_SOURCE_DIR}/build)

file(GLOB SOURCE_FILES ${SOURCE_DIR}/*.cpp)

project(GarnetExamples)

add_executable(server-tcp ${SOURCE_DIR}/server_tcp.cpp)
add_executable(client-tcp ${SOURCE_DIR}/client_tcp.cpp)
add_executable(server-tcp-class ${SOURCE_DIR}/server_tcp_class.cpp)
add_executable(client-tcp-class ${SOURCE_DIR}/client_tcp_class.cpp)
add_executable(server-udp ${SOURCE_DIR}/server_udp.cpp)
add_executable(client-udp ${SOURCE_DIR}/client_udp.cpp)
add_executable(server-udp-class ${SOURCE_DIR}/server_udp_class.cpp)
add_executable(client-udp-class ${SOURCE_DIR}/client_udp_class.cpp)

target_include_directories(server-tcp PUBLIC ${GNET_SOURCE_DIR})
target_include_directories(client-tcp PUBLIC ${GNET_SOURCE_DIR})
target_include_directories(server-tcp-class PUBLIC ${GNET_SOURCE_DIR})
target_include_directories(client-tcp-class PUBLIC ${GNET_SOURCE_DIR})
target_include_directories(server-udp PUBLIC ${GNET_SOURCE_DIR})
target_include_directories(client-udp PUBLIC ${GNET_SOURCE_DIR})
target_include_directories(server-udp-class PUBLIC ${GNET_SOURCE_DIR})
target_include_directories(client-udp-class PUBLIC ${GNET_SOURCE_DIR})

target_link_directories(server-tcp PUBLIC ${GNET_BUILD_DIR})
target_link_directories(client-tcp PUBLIC ${GNET_BUILD_DIR})
target_link_directories(server-tcp-class PUBLIC ${GNET_BUILD_DIR})
target_link_directories(client-tcp-class PUBLIC ${GNET_BUILD_DIR})
target_link_directories(server-udp PUBLIC ${GNET_BUILD_DIR})
target_link_directories(client-udp PUBLIC ${GNET_BUILD_DIR})
target_link_directories(server-udp-class PUBLIC ${GNET_BUILD_DIR})
target_link_directories(client-udp-class PUBLIC ${GNET_BUILD_DIR})

if(WIN32)
    target_link_libraries(server-tcp        garnet ws2_32)
    target_link_libraries(client-tcp        garnet ws2_32)
    target_link_libraries(server-tcp-class  garnet ws2_32)
    target_link_libraries(client-tcp-class  garnet ws2_32)
    target_link_libraries(server-udp        garnet ws2_32)
    target_link_libraries(client-udp        garnet ws2_32)
    target_link_libraries(server-udp-class  garnet ws2_32)
    target_link_libraries(client-udp-class  garnet ws2_32)

else()
    target_link_libraries(server-tcp        garnet)
    target_link_libraries(client-tcp        garnet)
    target_link_libraries(server-tcp-class  garnet)
    target_link_libraries(client-tcp-class  garnet)
    target_link_libraries(server-udp        garnet)
    target_link_libraries(client-udp        garnet)
    target_link_libraries(server-udp-class  garnet)
    target_link_libraries(client-udp-class  garnet)

endif()
