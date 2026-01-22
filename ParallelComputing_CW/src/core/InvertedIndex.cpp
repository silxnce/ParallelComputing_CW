#include "InvertedIndex.h"
#include "Tokenizer.h"
#include <algorithm>
#include <iterator>
#include <mutex>

using namespace std;

InvertedIndex::InvertedIndex() {}
InvertedIndex::~InvertedIndex() {}

void InvertedIndex::add_document(const string& doc_id, const string& text) {
    vector<string> tokens = Tokenizer::tokenize(text);
    unique_lock lock(index_mutex_);

    documents_.insert(doc_id);

    for (size_t position = 0; position < tokens.size(); ++position) {
        if (!tokens[position].empty()) {
            add_term(tokens[position], doc_id, position);
        }
    }
}

static set<string> extract_docs(
    const unordered_map<string, vector<size_t>>& postings
) {
    set<string> docs;
    for (const auto& [doc_id, _] : postings) {
        docs.insert(doc_id);
    }
    return docs;
}

vector<string> InvertedIndex::search(const string& term) const {
    shared_lock lock(index_mutex_);

    auto it = index_.find(Tokenizer::normalize(term));
    if (it == index_.end()) {
        return {};
    }

    vector<string> results;
    results.reserve(it->second.size());

    for (const auto& doc_pair : it->second) {
        results.push_back(doc_pair.first);
    }

    return results;
}

vector<string> InvertedIndex::search_and(const vector<string>& terms) const {
    if (terms.empty()) {
        return {};
    }

    shared_lock lock(index_mutex_);

    auto it = index_.find(Tokenizer::normalize(terms[0]));
    if (it == index_.end())
    {
        return {};
    }

    set<string> result = extract_docs(it->second);

    for (size_t i = 1; i < terms.size() && !result.empty(); ++i) {
        auto term_it = index_.find(Tokenizer::normalize(terms[i]));
        if (term_it == index_.end()) {
            return {};
        }

        set<string> current = extract_docs(term_it->second);
        set<string> intersection;

        set_intersection(
            result.begin(), result.end(),
            current.begin(), current.end(),
            inserter(intersection, intersection.begin())
        );
        result.swap(intersection);
    }

    return {result.begin(), result.end()};
}

vector<string> InvertedIndex::search_or(const vector<string>& terms) const {
    shared_lock lock(index_mutex_);
    set<string> result_set;

    for (const auto& term : terms) {
        auto it = index_.find(Tokenizer::normalize(term));
        if (it != index_.end()) {
            for (const auto& doc_pair : it->second) {
                result_set.insert(doc_pair.first);
            }
        }
    }

    return {result_set.begin(), result_set.end()};
}

size_t InvertedIndex::get_document_count() const {
    shared_lock lock(index_mutex_);
    return documents_.size();
}

size_t InvertedIndex::get_term_count() const {
    shared_lock lock(index_mutex_);
    return index_.size();
}

void InvertedIndex::clear() {
    unique_lock lock(index_mutex_);
    index_.clear();
    documents_.clear();
}

size_t InvertedIndex::get_term_frequency(const string& term) const {
    shared_lock lock(index_mutex_);

    auto it = index_.find(Tokenizer::normalize(term));
    if (it != index_.end()) {
        return it->second.size();
    }
    return 0;
}

void InvertedIndex::add_term(const string& term, const string& doc_id, size_t position) {
    index_[term][doc_id].push_back(position);
}