#include "Tokenizer.h"
#include <sstream>

using namespace std;

vector<string> Tokenizer::tokenize(const string& text) {
    vector<string> tokens;
    string current_token;

    for (char c : text) {
        if (is_valid_char(c)) {
            current_token += c;
        } else {
            if (!current_token.empty()) {
                tokens.push_back(normalize(current_token));
                current_token.clear();
            }
        }
    }

    if (!current_token.empty()) {
        tokens.push_back(normalize(current_token));
    }

    return tokens;
}

string Tokenizer::normalize(const string& token) {
    string normalized;
    normalized.reserve(token.size());

    for (char c : token) {
        normalized += tolower(static_cast<unsigned char>(c));
    }

    return normalized;
}

bool Tokenizer::is_valid_char(char c) {
    return isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '\'';
}