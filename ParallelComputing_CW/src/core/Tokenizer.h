#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

class Tokenizer {
public:
    static std::vector<std::string> tokenize(const std::string& text);
    static std::string normalize(const std::string& token);

private:
    static bool is_valid_char(char c);
};

#endif //TOKENIZER_H
