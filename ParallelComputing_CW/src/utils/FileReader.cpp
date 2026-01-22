#include "FileReader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace std::filesystem;

string FileReader::read_file(const string& file_path) {
    ifstream file(file_path);

    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + file_path);
    }

    stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

vector<string> FileReader::get_files_in_directory(
    const string& directory,
    const string& extension
) {
    vector<string> files;

    try {
        for (const auto& entry : recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                string path = entry.path().string();

                if (extension.empty() || entry.path().extension() == extension) {
                    files.push_back(path);
                }
            }
        }
    } catch (const filesystem_error& e) {
        throw runtime_error("Error reading directory: " + string(e.what()));
    }

    return files;
}

bool FileReader::file_exists(const string& file_path) {
    return exists(file_path);
}

string FileReader::get_filename(const string& file_path) {
    path path(file_path);
    return path.filename().string();
}