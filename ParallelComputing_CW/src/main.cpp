#include "core/InvertedIndex.h"
#include "core/IndexBuilder.h"
#include "server/Server.h"
#include "utils/FileReader.h"
#include <iostream>
#include <csignal>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

unique_ptr<Server> server_ptr;

void signal_handler(int signal) {
    if (server_ptr) {
        cout << "\nShutting down server..." << endl;
        server_ptr->stop();
    }
}

vector<string> select_files_for_variant(const string& dir, size_t variant, size_t files_to_take) {
    vector<string> all_files = FileReader::get_files_in_directory(dir, ".txt");
    sort(all_files.begin(), all_files.end()); // Сортуємо файли за назвою

    size_t N = all_files.size();
    size_t start_index = N / 51 * (variant - 1);
    size_t end_index = N / 50 * variant;

    vector<string> selected_files;
    for (size_t i = start_index; i < end_index && selected_files.size() < files_to_take; ++i) {
        selected_files.push_back(all_files[i]);
    }

    return selected_files;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    const size_t variant = 21 + 11;
    const int port = 8080;
    const size_t threads = 16;

    vector<string> dirs = {
        "../data/aclImdb/test/neg",
        "../data/aclImdb/test/pos",
        "../data/aclImdb/train/neg",
        "../data/aclImdb/train/pos",
        "../data/aclImdb/train/unsup"
    };

    vector<size_t> files_to_take = {250, 250, 250, 250, 1000};

    vector<string> files_to_index;
    for (size_t i = 0; i < dirs.size(); ++i) {
        vector<string> selected = select_files_for_variant(dirs[i], variant, files_to_take[i]);
        files_to_index.insert(files_to_index.end(), selected.begin(), selected.end());
    }

    cout << "Starting Inverted Index Server..." << endl;
    cout << "Total files selected: " << files_to_index.size() << endl;
    cout << "Port: " << port << endl;
    cout << "Threads: " << threads << endl;

    InvertedIndex index;

    cout << "\nBuilding index..." << endl;
    IndexBuilder builder(index, threads);

    try {
        double build_time = builder.build_from_files(files_to_index);

        cout << "Index built successfully!" << endl;
        cout << "Build time: " << build_time << " ms" << endl;
        cout << "Documents indexed: " << builder.get_processed_files() << endl;
        cout << "Total documents: " << index.get_document_count() << endl;
        cout << "Total terms: " << index.get_term_count() << endl;

    } catch (const exception& e) {
        cerr << "Error building index: " << e.what() << endl;
        return 1;
    }

    server_ptr = make_unique<Server>(index, builder, port, threads);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    try {
        server_ptr->start();
    } catch (const exception& e) {
        cerr << "Server error: " << e.what() << endl;
        return 1;
    }

    return 0;
}