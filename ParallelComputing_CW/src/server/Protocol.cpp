#include "Protocol.h"
#include <sstream>

using namespace std;

bool Protocol::parse_command(
    const string& message,
    string& command,
    vector<string>& args
    ) {
    args.clear();

    istringstream iss(message);

    if (!(iss >> command)) {
        return false;
    }

    string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    return true;
}

string Protocol::create_result_response(const vector<string>& results) {
    ostringstream oss;
    oss << RESP_RESULT << " " << results.size();

    for (const auto& result : results) {
        oss << " " << result;
    }

    oss << "\n";
    return oss.str();
}

string Protocol::create_stats_response(size_t doc_count, size_t term_count) {
    ostringstream oss;
    oss << RESP_STATS << " " << doc_count << " " << term_count << "\n";
    return oss.str();
}

string Protocol::create_term_freq_response(const string& term, size_t frequency) {
    ostringstream oss;
    oss << RESP_TERM_FREQ << " " << term << " " << frequency << "\n";
    return oss.str();
}

string Protocol::create_error_response(const string& error_message) {
    return string(RESP_ERROR) + " " + error_message + "\n";
}

string Protocol::create_ok_response() {
    return string(RESP_OK) + "\n";
}