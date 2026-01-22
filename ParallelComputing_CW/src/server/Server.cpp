#include "Server.h"
#include "Protocol.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>
#include <stdexcept>

#pragma comment(lib, "ws2_32.lib")

Server::Server(
    InvertedIndex& index,
    IndexBuilder& builder,
    int port,
    size_t thread_count)
    :   index_(index),
        builder_(builder),
        port_(port),
        server_socket_(-1),
        running_(false)
{
    thread_pool_ = std::make_unique<ThreadPool>(thread_count);
}

Server::~Server() {
    stop();
}

void Server::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    server_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_ == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    setsockopt(
        server_socket_,
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<const char*>(&opt),
        sizeof(opt)
        );

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    if (bind(server_socket_, reinterpret_cast<sockaddr*>(&address),
         sizeof(address)) == SOCKET_ERROR) {
        closesocket(server_socket_);
        WSACleanup();
        throw std::runtime_error("Bind failed");
         }

    if (listen(server_socket_, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(server_socket_);
        WSACleanup();
        throw std::runtime_error("Listen failed");
    }

    running_ = true;
    std::cout << "Server started on port " << port_ << std::endl;

    while (running_) {
        sockaddr_in client_address{};
        int client_len = sizeof(client_address);

        SOCKET client_socket =
            accept(server_socket_,
                   reinterpret_cast<sockaddr*>(&client_address),
                   &client_len);

        if (client_socket == INVALID_SOCKET) {
            if (running_) {
                std::cerr << "Accept failed" << std::endl;
            }
            continue;
        }

        thread_pool_->enqueue([this, client_socket]() {
            handle_client(client_socket);
        });
    }
}

void Server::stop() {
    if (running_) {
        running_ = false;

        if (server_socket_ != INVALID_SOCKET) {
            closesocket(server_socket_);
            server_socket_ = INVALID_SOCKET;
        }

        WSACleanup();
        std::cout << "Server stopped" << std::endl;
    }
}

bool Server::is_running() const {
    return running_;
}

void Server::handle_client(SOCKET client_socket) {
    const size_t BUFFER_SIZE = 100000;
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        size_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_read <= 0) {
            break; // Клієнт відключився
        }

        std::string message(buffer);

        std::string command;
        std::vector<std::string> args;

        if (!Protocol::parse_command(message, command, args)) {
            std::string error_response = Protocol::create_error_response("Invalid command format");
            send(client_socket, error_response.c_str(), error_response.length(), 0);
            continue;
        }

        if (command == Protocol::CMD_EXIT) {
            std::string ok_response = Protocol::create_ok_response();
            send(client_socket, ok_response.c_str(), ok_response.length(), 0);
            break;
        }

        std::string response = process_command(command, args);
        send(client_socket, response.c_str(), response.length(), 0);
    }

    closesocket(client_socket);
}

std::string Server::process_command(
    const std::string& command,
    const std::vector<std::string>& args
    ) {
    try {
        if (command == Protocol::CMD_SEARCH) {
            if (args.empty()) {
                return Protocol::create_error_response("SEARCH requires a term");
            }

            std::vector<std::string> results = index_.search(args[0]);
            return Protocol::create_result_response(results);

        } else if (command == Protocol::CMD_SEARCH_AND) {
            if (args.empty()) {
                return Protocol::create_error_response("SEARCH_AND requires at least one term");
            }

            std::vector<std::string> results = index_.search_and(args);
            return Protocol::create_result_response(results);

        } else if (command == Protocol::CMD_SEARCH_OR) {
            if (args.empty()) {
                return Protocol::create_error_response("SEARCH_OR requires at least one term");
            }

            std::vector<std::string> results = index_.search_or(args);
            return Protocol::create_result_response(results);

        } else if (command == Protocol::CMD_STATS) {
            size_t doc_count = index_.get_document_count();
            size_t term_count = index_.get_term_count();
            return Protocol::create_stats_response(doc_count, term_count);

        } else if (command == Protocol::CMD_TERM_FREQ) {
            if (args.empty()) {
                return Protocol::create_error_response("TERM_FREQ requires a term");
            }

            size_t frequency = index_.get_term_frequency(args[0]);
            return Protocol::create_term_freq_response(args[0], frequency);

        } else if (command == Protocol::CMD_ADD_FILE) {
            if (args.empty()) {
                return Protocol::create_error_response("ADD_FILE requires file path");
            }

            const std::string& file_path = args[0];

            if (!FileReader::file_exists(file_path)) {
                return Protocol::create_error_response("File does not exist");
            }

            builder_.add_file(file_path);

            return Protocol::create_ok_response();

        } else {
            return Protocol::create_error_response("Unknown command: " + command);
        }

    } catch (const std::exception& e) {
        return Protocol::create_error_response(e.what());
    }
}