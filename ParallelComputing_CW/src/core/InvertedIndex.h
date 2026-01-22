#ifndef INVERTEDINDEX_H
#define INVERTEDINDEX_H

#include <string>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <set>

class InvertedIndex {
public:
    InvertedIndex();
    ~InvertedIndex();

    void add_document(const std::string& doc_id, const std::string& text);
    std::vector<std::string> search(const std::string& term) const;
    std::vector<std::string> search_and(const std::vector<std::string>& terms) const;
    std::vector<std::string> search_or(const std::vector<std::string>& terms) const;
    size_t get_document_count() const;
    size_t get_term_count() const;
    void clear();
    size_t get_term_frequency(const std::string& term) const;

private:
    std::unordered_map<
        std::string,
        std::unordered_map<std::string, std::vector<size_t>>
    > index_;
    std::set<std::string> documents_;
    mutable std::shared_mutex index_mutex_;

    void add_term(const std::string& term, const std::string& doc_id, size_t position);
};

#endif //INVERTEDINDEX_H
