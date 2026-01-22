#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <vector>

class Protocol {
public:
    static constexpr const char* CMD_SEARCH = "SEARCH";
    static constexpr const char* CMD_SEARCH_AND = "SEARCH_AND";
    static constexpr const char* CMD_SEARCH_OR = "SEARCH_OR";
    static constexpr const char* CMD_STATS = "STATS";
    static constexpr const char* CMD_TERM_FREQ = "TERM_FREQ";
    static constexpr const char* CMD_ADD_FILE = "ADD_FILE";
    static constexpr const char* CMD_EXIT = "EXIT";

    static constexpr const char* RESP_OK = "OK";
    static constexpr const char* RESP_RESULT = "RESULT";
    static constexpr const char* RESP_STATS = "STATS";
    static constexpr const char* RESP_TERM_FREQ = "TERM_FREQ";
    static constexpr const char* RESP_ADD_FILE = "ADD_FILE";
    static constexpr const char* RESP_ERROR = "ERROR";

    static bool parse_command(const std::string& message, std::string& command, std::vector<std::string>& args);
    static std::string create_result_response(const std::vector<std::string>& results);
    static std::string create_stats_response(size_t doc_count, size_t term_count);
    static std::string create_term_freq_response(const std::string& term, size_t frequency);
    static std::string create_error_response(const std::string& error_message);
    static std::string create_ok_response();
};

#endif //PROTOCOL_H
