cmake_minimum_required(VERSION 3.5.1)

project(HelloWorldGrpc)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_GRPC_GRPCPP gRPC::grpc++)

add_executable(server server.cpp
    helloworld.pb.cc helloworld.grpc.pb.cc)
target_link_libraries(server
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})

add_executable(client client.cpp
    helloworld.pb.cc helloworld.grpc.pb.cc)
target_link_libraries(client
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
