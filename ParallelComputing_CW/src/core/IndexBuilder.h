#ifndef INDEXBUILDER_H
#define INDEXBUILDER_H

#include "./InvertedIndex.h"
#include "./ThreadPool.h"
#include <string>
#include <vector>
#include <atomic>
#include <chrono>

class IndexBuilder {
public:
    explicit IndexBuilder(InvertedIndex& index, size_t thread_count = std::thread::hardware_concurrency());

    double build_from_directory(const std::string& directory);
    double build_from_files(const std::vector<std::string>& files);
    void add_file(const std::string& file_path);
    size_t get_processed_files() const;
    size_t get_thread_count() const;

private:
    InvertedIndex& index_;
    ThreadPool thread_pool_;
    std::atomic<size_t> processed_files_;

    void process_file(const std::string& file_path);
};

#endif //INDEXBUILDER_H