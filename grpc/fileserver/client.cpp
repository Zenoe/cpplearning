#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <grpcpp/grpcpp.h>
#include "file_service.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::Status;
using fileservice::FileService;
using fileservice::FileChunk;
using fileservice::FileRequest;
using fileservice::FileResponse;
using fileservice::ListRequest;
using fileservice::FileList;

namespace fs = std::filesystem;

class FileClient {
private:
    std::unique_ptr<FileService::Stub> stub_;
    static constexpr size_t CHUNK_SIZE = 64 * 1024; // 64KB chunks

public:
    FileClient(std::shared_ptr<Channel> channel)
        : stub_(FileService::NewStub(channel)) {}

    bool UploadFile(const std::string& local_path, const std::string& remote_filename = "") {
        if (!fs::exists(local_path)) {
            std::cout << "File not found: " << local_path << std::endl;
            return false;
        }

        std::string filename = remote_filename.empty() ?
            fs::path(local_path).filename().string() : remote_filename;

        std::ifstream input_file(local_path, std::ios::binary);
        if (!input_file.is_open()) {
            std::cout << "Failed to open file: " << local_path << std::endl;
            return false;
        }

        ClientContext context;
        FileResponse response;
        std::unique_ptr<ClientWriter<FileChunk>> writer(
            stub_->UploadFile(&context, &response));

        char buffer[CHUNK_SIZE];
        int64_t offset = 0;

        std::cout << "Uploading file: " << filename << std::endl;

        while (input_file.read(buffer, CHUNK_SIZE) || input_file.gcount() > 0) {
            FileChunk chunk;
            chunk.set_filename(filename);
            chunk.set_data(buffer, input_file.gcount());
            chunk.set_offset(offset);
            chunk.set_is_last(input_file.eof());

            if (!writer->Write(chunk)) {
                std::cout << "Failed to write chunk" << std::endl;
                break;
            }

            offset += input_file.gcount();

            if (input_file.eof()) {
                break;
            }
        }

        input_file.close();
        writer->WritesDone();

        Status status = writer->Finish();

        if (status.ok()) {
            std::cout << "Upload successful: " << response.message()
                     << " (Size: " << response.file_size() << " bytes)" << std::endl;
            return true;
        } else {
            std::cout << "Upload failed: " << status.error_message() << std::endl;
            return false;
        }
    }

    bool DownloadFile(const std::string& remote_filename, const std::string& local_path = "") {
        std::string output_path = local_path.empty() ? remote_filename : local_path;

        FileRequest request;
        request.set_filename(remote_filename);

        ClientContext context;
        std::unique_ptr<ClientReader<FileChunk>> reader(
            stub_->DownloadFile(&context, request));

        std::ofstream output_file(output_path, std::ios::binary);
        if (!output_file.is_open()) {
            std::cout << "Failed to create output file: " << output_path << std::endl;
            return false;
        }

        FileChunk chunk;
        int64_t total_size = 0;

        std::cout << "Downloading file: " << remote_filename << std::endl;

        while (reader->Read(&chunk)) {
            output_file.write(chunk.data().c_str(), chunk.data().size());
            total_size += chunk.data().size();

            if (chunk.is_last()) {
                break;
            }
        }

        output_file.close();

        Status status = reader->Finish();

        if (status.ok()) {
            std::cout << "Download successful: " << output_path
                     << " (Size: " << total_size << " bytes)" << std::endl;
            return true;
        } else {
            std::cout << "Download failed: " << status.error_message() << std::endl;
            return false;
        }
    }

    bool ListFiles(const std::string& directory = "") {
        ListRequest request;
        request.set_directory(directory);

        ClientContext context;
        FileList response;

        Status status = stub_->ListFiles(&context, request, &response);

        if (status.ok()) {
            std::cout << "\n--- File List ---" << std::endl;
            for (const auto& file : response.files()) {
                std::string type = file.is_directory() ? "[DIR]" : "[FILE]";
                std::cout << type << " " << file.name();
                if (!file.is_directory()) {
                    std::cout << " (" << file.size() << " bytes)";
                }
                std::cout << std::endl;
            }
            std::cout << "--- Total: " << response.files_size() << " items ---\n" << std::endl;
            return true;
        } else {
            std::cout << "List files failed: " << status.error_message() << std::endl;
            return false;
        }
    }

    bool DeleteFile(const std::string& filename) {
        FileRequest request;
        request.set_filename(filename);

        ClientContext context;
        FileResponse response;

        Status status = stub_->DeleteFile(&context, request, &response);

        if (status.ok()) {
            if (response.success()) {
                std::cout << "Delete successful: " << response.message() << std::endl;
                return true;
            } else {
                std::cout << "Delete failed: " << response.message() << std::endl;
                return false;
            }
        } else {
            std::cout << "Delete failed: " << status.error_message() << std::endl;
            return false;
        }
    }
};

void PrintUsage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  upload <local_file> [remote_name]" << std::endl;
    std::cout << "  download <remote_file> [local_path]" << std::endl;
    std::cout << "  list [directory]" << std::endl;
    std::cout << "  delete <remote_file>" << std::endl;
    std::cout << "  help" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    FileClient client(channel);

    std::string command = argv[1];

    if (command == "upload" && argc >= 3) {
        std::string local_file = argv[2];
        std::string remote_name = argc >= 4 ? argv[3] : "";
        client.UploadFile(local_file, remote_name);
    }
    else if (command == "download" && argc >= 3) {
        std::string remote_file = argv[2];
        std::string local_path = argc >= 4 ? argv[3] : "";
        client.DownloadFile(remote_file, local_path);
    }
    else if (command == "list") {
        std::string directory = argc >= 3 ? argv[2] : "";
        client.ListFiles(directory);
    }
    else if (command == "delete" && argc >= 3) {
        std::string filename = argv[2];
        client.DeleteFile(filename);
    }
    else if (command == "help") {
        PrintUsage();
    }
    else {
        std::cout << "Unknown command or insufficient arguments." << std::endl;
        PrintUsage();
        return 1;
    }

    return 0;
}
