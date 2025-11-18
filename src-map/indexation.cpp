#include <unordered_map>
#include "tokenizer.cpp"

class doc_t {
private:
    std::string path;
    //trie* content = nullptr;
    std::unordered_map<std::string, int> tf_map;
public:
    doc_t() = delete;

    doc_t(std::string path, std::string& text) {
        //content = new trie;
        this->path = path;
        std::vector<std::string> tokens = get_tokens(text);
        for (auto& t : tokens) {
            tf_map[t] += 1;
            //content->insert(t);
        }
    }

    doc_t(const doc_t& other) {
        this->path = other.path;
        //this->content = other.content;
        this->tf_map = other.tf_map;
    }

    doc_t(doc_t&& other) noexcept {
        this->path = other.path;
        //this->content = other.content;
        this->tf_map = other.tf_map;
    }

    ~doc_t() {
        //delete content;
        //content = nullptr;
    }

    std::string get_path() const { return path; }

    std::unordered_map<std::string, int> get_tf_map() const { return tf_map; }

    size_t get_bytes_count() {
        size_t bytes = 0;
        bytes += sizeof(path) + path.capacity();
        bytes += sizeof(tf_map);
        for(auto& p : tf_map) {
            size_t temp = 0;
            temp += sizeof(p.first) + p.first.capacity();
            temp += sizeof(p.second);
            bytes += 24 * temp;
        }
        return bytes;
    }
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
