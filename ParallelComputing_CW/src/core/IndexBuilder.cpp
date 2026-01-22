#include "IndexBuilder.h"
#include "../utils/FileReader.h"
#include <iostream>

using namespace std;

IndexBuilder::IndexBuilder(InvertedIndex& index, size_t thread_count)
    : index_(index), thread_pool_(thread_count), processed_files_(0) {}

double IndexBuilder::build_from_directory(const string& directory) {
    // Отримуємо список всіх файлів
    return build_from_files(
        FileReader::get_files_in_directory(directory, ".txt")
    );
}

double IndexBuilder::build_from_files(const vector<string>& files) {
    processed_files_ = 0;
    auto start_time = chrono::high_resolution_clock::now();

    // Створюємо futures для всіх задач
    vector<future<void>> futures;
    futures.reserve(files.size());

    for (const auto& file_path : files) {
        futures.emplace_back(
            thread_pool_.enqueue([this, file_path]() {
                process_file(file_path);
            })
        );
    }

    // Чекаємо завершення всіх задач
    for (auto& future : futures) {
        future.get();
    }

    auto end_time = chrono::high_resolution_clock::now();
    return chrono::duration<double, milli>(end_time - start_time).count();
}

void IndexBuilder::add_file(const string& file_path) {
    thread_pool_.enqueue([this, file_path]() {
        process_file(file_path);
    });
}

size_t IndexBuilder::get_processed_files() const {
    return processed_files_.load();
}

size_t IndexBuilder::get_thread_count() const {
    return thread_pool_.get_thread_count();
}

void IndexBuilder::process_file(const string& file_path) {
    try {
        string content = FileReader::read_file(file_path);
        index_.add_document(file_path, content);
        ++processed_files_;
    } catch (const exception& e) {
        cerr << "Error processing file " << file_path << ": " << e.what() << endl;
    }
}