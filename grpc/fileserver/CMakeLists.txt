cmake_minimum_required(VERSION 3.16)
project(FileServerClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED) # necessary
# find_package(PkgConfig REQUIRED) # not needed, not use pkg_check_modules

# Generate protobuf and gRPC files
set(PROTO_FILES file_service.proto)
set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto-src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
set(GRPC_SRCS ${PROTO_SRC_DIR}/file_service.grpc.pb.cc)
set(GRPC_HDRS ${PROTO_SRC_DIR}/file_service.grpc.pb.h)

add_custom_command(
    OUTPUT ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND protoc
    ARGS --grpc_out ${PROTO_SRC_DIR}
         --cpp_out ${PROTO_SRC_DIR}
         --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
         --proto_path ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/file_service.proto
    DEPENDS ${PROTO_FILES}
)

# include_directories(${PROTO_SRC_DIR})

# Server
add_executable(file_server
    server.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
# Use modern gRPC targets: protobuf::libprotobuf and gRPC::grpc++
target_link_libraries(file_server
    protobuf::libprotobuf
    gRPC::grpc++
    # ${GRPC_LIBRARIES}
    # ${Protobuf_LIBRARIES}
    # pthread
)

# Client
add_executable(file_client
    client.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(file_client
    protobuf::libprotobuf
    gRPC::grpc++
    # ${GRPC_LIBRARIES}
    # ${Protobuf_LIBRARIES}
)
