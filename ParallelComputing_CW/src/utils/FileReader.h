#ifndef FILEREADER_H
#define FILEREADER_H

#include <string>
#include <vector>
#include <filesystem>

class FileReader {
public:
    static std::string read_file(const std::string& file_path);
    static std::vector<std::string> get_files_in_directory(
        const std::string& directory,
        const std::string& extension = ""
    );
    static bool file_exists(const std::string& file_path);
    static std::string get_filename(const std::string& file_path);
};

#endif //FILEREADER_H
