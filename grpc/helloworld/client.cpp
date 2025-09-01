#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "helloworld.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::HelloWorld;
using helloworld::HelloRequest;
using helloworld::HelloReply;

class HelloWorldClient {
 public:
  HelloWorldClient(std::shared_ptr<Channel> channel)
      : stub_(HelloWorld::NewStub(channel)) {}

  std::string SayHello(const std::string& user) {
    HelloRequest request;
    request.set_name(user);

    HelloReply reply;

    ClientContext context;
    Status status = stub_->SayHello(&context, request, &reply);

    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << "RPC failed, code: " << status.error_code()
                << ", message: " << status.error_message() << std::endl;
      return "RPC failed";
    }

  }

 private:
  std::unique_ptr<HelloWorld::Stub> stub_;
};

int main(int argc, char** argv) {
  HelloWorldClient client(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  std::string user("World");
  std::string reply = client.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;

  return 0;
}
