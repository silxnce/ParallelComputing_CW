#ifndef SERVER_H
#define SERVER_H

#include "../core/InvertedIndex.h"
#include "../core/IndexBuilder.h"
#include "../core/ThreadPool.h"
#include "../utils/FileReader.h"
#include <winsock2.h>
#include <string>
#include <atomic>
#include <memory>

class Server {
public:
    explicit Server(
        InvertedIndex& index,
        IndexBuilder& builder,
        int port = 8080,
        size_t thread_count = 4
        );
    ~Server();

    void start();
    void stop();
    bool is_running() const;

private:
    InvertedIndex& index_;
    IndexBuilder& builder_;
    int port_;
    SOCKET server_socket_;
    std::atomic<bool> running_;
    std::unique_ptr<ThreadPool> thread_pool_;

    void handle_client(SOCKET client_socket);
    std::string process_command(
        const std::string& command,
        const std::vector<std::string>& args
    );
};

#endif //SERVER_H
