#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <grpcpp/grpcpp.h>
#include "file_service.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;
using fileservice::FileService;
using fileservice::FileChunk;
using fileservice::FileRequest;
using fileservice::FileResponse;
using fileservice::ListRequest;
using fileservice::FileList;
using fileservice::FileInfo;

namespace fs = std::filesystem;

class FileServiceImpl final : public FileService::Service {
private:
    std::string server_root_dir = "./server_files/";
    static constexpr size_t CHUNK_SIZE = 64 * 1024; // 64KB chunks

public:
    FileServiceImpl() {
        // Create server directory if it doesn't exist
        fs::create_directories(server_root_dir);
    }

    Status UploadFile(ServerContext* context,
                     ServerReader<FileChunk>* reader,
                     FileResponse* response) override {

        FileChunk chunk;
        std::ofstream output_file;
        std::string filename;
        int64_t total_size = 0;
        bool first_chunk = true;

        while (reader->Read(&chunk)) {
            if (first_chunk) {
                filename = server_root_dir + chunk.filename();
                output_file.open(filename, std::ios::binary);

                if (!output_file.is_open()) {
                    response->set_success(false);
                    response->set_message("Failed to create file: " + chunk.filename());
                    return Status::OK;
                }
                first_chunk = false;
                std::cout << "Starting upload: " << chunk.filename() << std::endl;
            }

            output_file.write(chunk.data().c_str(), chunk.data().size());
            total_size += chunk.data().size();

            if (chunk.is_last()) {
                break;
            }
        }

        output_file.close();

        response->set_success(true);
        response->set_message("File uploaded successfully");
        response->set_file_size(total_size);

        std::cout << "Upload completed: " << filename << " (" << total_size << " bytes)" << std::endl;
        return Status::OK;
    }

    Status DownloadFile(ServerContext* context,
                       const FileRequest* request,
                       ServerWriter<FileChunk>* writer) override {

        std::string filepath = server_root_dir + request->filename();

        if (!fs::exists(filepath)) {
            return Status(grpc::StatusCode::NOT_FOUND, "File not found");
        }

        std::ifstream input_file(filepath, std::ios::binary);
        if (!input_file.is_open()) {
            return Status(grpc::StatusCode::INTERNAL, "Failed to open file");
        }

        std::cout << "Starting download: " << request->filename() << std::endl;

        char buffer[CHUNK_SIZE];
        int64_t offset = 0;

        while (input_file.read(buffer, CHUNK_SIZE) || input_file.gcount() > 0) {
            FileChunk chunk;
            chunk.set_filename(request->filename());
            chunk.set_data(buffer, input_file.gcount());
            chunk.set_offset(offset);
            chunk.set_is_last(input_file.eof());

            if (!writer->Write(chunk)) {
                break;
            }

            offset += input_file.gcount();

            if (input_file.eof()) {
                break;
            }
        }

        input_file.close();
        std::cout << "Download completed: " << request->filename() << std::endl;
        return Status::OK;
    }

    Status ListFiles(ServerContext* context,
                    const ListRequest* request,
                    FileList* response) override {

        std::string directory = server_root_dir;
        if (!request->directory().empty()) {
            directory += request->directory();
        }

        try {
            for (const auto& entry : fs::directory_iterator(directory)) {
                FileInfo* file_info = response->add_files();
                file_info->set_name(entry.path().filename().string());
                file_info->set_is_directory(entry.is_directory());

                if (entry.is_regular_file()) {
                    file_info->set_size(fs::file_size(entry));
                } else {
                    file_info->set_size(0);
                }
            }
        } catch (const fs::filesystem_error& e) {
            return Status(grpc::StatusCode::NOT_FOUND, "Directory not found");
        }

        std::cout << "Listed " << response->files_size() << " files/directories" << std::endl;
        return Status::OK;
    }

    Status DeleteFile(ServerContext* context,
                     const FileRequest* request,
                     FileResponse* response) override {

        std::string filepath = server_root_dir + request->filename();

        try {
            if (fs::remove(filepath)) {
                response->set_success(true);
                response->set_message("File deleted successfully");
                std::cout << "Deleted file: " << request->filename() << std::endl;
            } else {
                response->set_success(false);
                response->set_message("File not found");
            }
        } catch (const fs::filesystem_error& e) {
            response->set_success(false);
            response->set_message("Error deleting file: " + std::string(e.what()));
        }

        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    FileServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "File server listening on " << server_address << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}
