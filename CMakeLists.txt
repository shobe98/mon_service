cmake_minimum_required(VERSION 3.13)
project(messages LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(protos)


add_executable(mon_service mon_service.cc)
target_link_libraries(mon_service LINK_PUBLIC stats_server_proto client)