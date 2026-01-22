#include <iostream>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class Client {
public:
    Client(const string& host, int port)
        : host_(host), port_(port), socket_(INVALID_SOCKET) {}

    ~Client() {
        disconnect();
        WSACleanup();
    }

    bool connect() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed\n";
            return false;
        }

        socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket_ == INVALID_SOCKET) {
            cerr << "Socket creation failed\n";
            return false;
        }

        sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port_);

        if (inet_pton(AF_INET, host_.c_str(), &server_address.sin_addr) != 1) {
            cerr << "Invalid IP address\n";
            return false;
        }

        if (::connect(
                socket_,
                reinterpret_cast<sockaddr*>(&server_address),
                sizeof(server_address)
            ) == SOCKET_ERROR) {

            cerr << "Connection failed\n";
            return false;
        }

        cout << "Connected to server "
                  << host_ << ":" << port_ << "\n";
        return true;
    }

    void disconnect() {
        if (socket_ != INVALID_SOCKET) {
            closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
    }

    string send_command(const string& command) {
        if (socket_ == INVALID_SOCKET)
            return "ERROR: Not connected\n";

        int sent = send(
            socket_,
            command.c_str(),
            static_cast<int>(command.size()),
            0
        );

        if (sent == SOCKET_ERROR)
            return "ERROR: Send failed\n";

        char buffer[100000];
        memset(buffer, 0, sizeof(buffer));

        int received = recv(
            socket_,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (received == SOCKET_ERROR || received == 0)
            return "ERROR: Receive failed\n";

        return string(buffer, received);
    }

private:
    string host_;
    int port_;
    SOCKET socket_;
};

void print_help() {
    cout << "\nAvailable commands:\n"
              << "  search <term>           - Search for documents containing the term\n"
              << "  search_and <t1> <t2>... - Search for documents containing all terms\n"
              << "  search_or <t1> <t2>...  - Search for documents containing any term\n"
              << "  stats                   - Get index statistics\n"
              << "  freq <term>             - Get term frequency\n"
              << "  add_file <server_path>  - Add file to index (server-side)\n"
              << "  help                    - Show this help\n"
              << "  exit                    - Exit the client\n";
}

int main(int argc, char* argv[]) {
    string host = "127.0.0.1";
    int port = 8080;

    if (argc > 1) host = argv[1];
    if (argc > 2) port = atoi(argv[2]);

    Client client(host, port);

    if (!client.connect())
        return 1;

    print_help();

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line))
            break;

        if (line.empty())
            continue;

        if (line == "help") {
            print_help();
            continue;
        }

        if (line == "exit") {
            client.send_command("EXIT");
            break;
        }

        string protocol_command;

        if (line.rfind("search_and", 0) == 0) {
            protocol_command = "SEARCH_AND" + line.substr(10);
        } else if (line.rfind("search_or", 0) == 0) {
            protocol_command = "SEARCH_OR" + line.substr(9);
        } else if (line.rfind("search", 0) == 0) {
            protocol_command = "SEARCH" + line.substr(6);
        } else if (line == "stats") {
            protocol_command = "STATS";
        } else if (line.rfind("freq", 0) == 0) {
            protocol_command = "TERM_FREQ" + line.substr(4);
        } else if (line.rfind("add_file", 0) == 0) {
            protocol_command = "ADD_FILE" + line.substr(8);
        } else {
            cout << "Unknown command. Type 'help'.\n";
            continue;
        }

        string response = client.send_command(protocol_command);
        cout << response;
    }

    cout << "Finishing the work of the C++ Client...\n";
    return 0;
}
