find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)


set(LIBRARY_NAME
    client
)


add_library(${LIBRARY_NAME})


target_sources(${LIBRARY_NAME}
    PUBLIC
        include/statsclient.h
    PRIVATE
        src/statsclient.cc
)

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        stats_server_proto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)


target_include_directories(${LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

