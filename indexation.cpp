#include <string>
#include "trie.cpp"

class doc_info {
private:
    std::string doc_name;
    //trie metadata;
    trie stemmed_doc;

public:
    doc_info() = delete;

    doc_info(std::string doc_name, trie stemmed_doc) {
        this->doc_name = doc_name;
        this->stemmed_doc = stemmed_doc;
    }

    ~doc_info() {}

    std::string get_name() { return doc_name; }

    bool contains(std::string stem) { return stemmed_doc.contains(stem); }

    size_t count(std::string stem) { return stemmed_doc.count(stem); }
};
