#include <string>
#include "trie.cpp"

class doc_info {
private:
    std::string doc_name; //�������� ����
    trie stemmed_doc;

public:
    doc_info() = delete;

    doc_info(std::string& doc_name, trie& stemmed_doc) { //�����������
        this->doc_name = doc_name;
        this->stemmed_doc = stemmed_doc;
    }

    ~doc_info() {}

    std::string& get_name() { return doc_name; } //������ ��� �������� ����

    trie get_trie() { return stemmed_doc; } //������ ��� ��������� �������

    bool contains(std::string& stem) { return stemmed_doc.contains(stem); } //�������� ��������������� ������

    size_t count(std::string& stem) { return stemmed_doc.count(stem); } //���������� ����� ���������� ������ � ���������

    size_t count(std::vector<std::string>& stems) { return stemmed_doc.count(stems); } //���������� ��������� ��������� ������� ����� � ���������
};