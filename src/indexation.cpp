#include <unordered_map>
#include "tokenizer.cpp"

class doc_t {
private:
    std::string path;
    trie* content = nullptr;
public:
    doc_t() = delete;

    doc_t(std::string path, std::string& text) {
        content = new trie;
        this->path = path;
        std::vector<std::string> tokens = get_tokens(text);
        for (auto& t : tokens) {
            content->insert(t);
        }
    }

    doc_t(const doc_t& other) {
        this->path = other.path;
        this->content = other.content;
    }

    doc_t(doc_t&& other) noexcept {
        this->path = other.path;
        this->content = other.content;
    }

    ~doc_t() {
        delete content;
        content = nullptr;
    }

    const trie* get_content() { return content; }

    std::string get_path() const { return path; }
};

class doc_list {
private:
	std::vector<doc_t*> list = {};
public:
	doc_list() {}

	~doc_list() {
		for(auto& d : list) {
			delete d;
		}
	}

	doc_t* operator[](size_t pos) { return list[pos]; }

	void push_back(doc_t* d) { list.push_back(d); }

	bool empty() { return list.empty(); }

	size_t size() { return list.size(); }

	size_t capacity() { return list.capacity(); }

    doc_t* back() { return list.back(); }
};