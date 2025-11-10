#include <unordered_map>
#include "tokenizer.cpp"

class doc_t {
private:
    std::string path;
    trie* content = nullptr;
    //std::unordered_map<std::string, int> tf_map;
public:
    doc_t() = delete;

    doc_t(std::string path, std::string& text) {
        content = new trie;
        this->path = path;
        std::vector<std::string> tokens = get_tokens(text);
        /*
        for (auto& t : tokens) {
            tf_map[t] += 1;
            content->insert(t);
        }
        */
    }

    doc_t(const doc_t& other) {
        this->path = other.path;
        this->content = other.content;
        //this->tf_map = other.tf_map;
    }

    doc_t(doc_t&& other) noexcept {
        this->path = other.path;
        this->content = other.content;
        //this->tf_map = other.tf_map;
    }

    ~doc_t() {
        delete content;
    }

    const trie* get_content() { return content; }
    std::string get_path() const { return path; }

    //std::unordered_map<std::string, int> get_tf_map() const { return tf_map; }
};
