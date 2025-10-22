#include <string>
#include "trie.cpp"

class doc_info {
private:
    std::string doc_name; //название дока
    trie stemmed_doc;

public:
    doc_info() = delete;

    doc_info(std::string& doc_name, trie& stemmed_doc) { //конструктор
        this->doc_name = doc_name;
        this->stemmed_doc = stemmed_doc;
    }

    ~doc_info() {}

    std::string& get_name() { return doc_name; } //геттер для названия дока

    trie get_trie() { return stemmed_doc; } //геттер для хранилища стеммов

    bool contains(std::string& stem) { return stemmed_doc.contains(stem); } //содержит соответствующую стемму

    size_t count(std::string& stem) { return stemmed_doc.count(stem); } //количество одной конкретной стеммы в документе

    size_t count(std::vector<std::string>& stems) { return stemmed_doc.count(stems); } //количество вхождения элементов вектора стемм в документе
};