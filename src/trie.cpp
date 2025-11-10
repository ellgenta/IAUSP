#include <string>
#include <vector>

#define TRIE
#define UNIT_TESTS
#define CH_SIZE  26

struct trie_node {
    trie_node* ch[CH_SIZE];
    size_t count = 0;

    trie_node() {
        for(int i = 0; i < CH_SIZE; i++) {
            ch[i] = nullptr;
        }
    }

    bool is_leaf() {
        if (this == nullptr) {
            return true;
        }

        for (size_t i = 0; i < CH_SIZE; i++) {
            if (this->ch[i] != nullptr) {
                return false;
            }
        }

        return true;
    }
};

class trie {
private:
    trie_node* root = nullptr;
    size_t ch_count = 0;

    inline size_t get_index(char& sym) { return (unsigned int)sym % (unsigned int)'a'; }

    trie_node* _find(trie_node* root, std::string& stem, size_t i) {
        if (root == nullptr) {
            return nullptr;
        }

        if (i >= stem.size()) {
            return root;
        }

        if (root->ch[get_index(stem[i])] != nullptr) {
            return _find(root->ch[get_index(stem[i])], stem, i + 1);
        }

        return nullptr;
    }

    void _insert(trie_node* root, std::string& stem, size_t i) {
        if (root == nullptr || i == stem.size()) {
            return;
        }

        if (root->ch[get_index(stem[i])] != nullptr) {
            _insert(root->ch[get_index(stem[i])], stem, i + 1);
        }
        else {
            if (root == this->root) {
                ch_count += 1;
            }
            _push_prefix(root, stem, i);
        }
    }

    void _erase(trie_node* root, std::string& stem, size_t i) {
        if (root == nullptr) {
            return;
        }

        if (i == stem.size()) {
            root->count -= 1;
            return;
        }

        if (root->ch[get_index(stem[i])] != nullptr) {
            _erase(root->ch[get_index(stem[i])], stem, i + 1);
            if (root->ch[get_index(stem[i])]->is_leaf() == true) {
                delete root->ch[i];
                root->ch[get_index(stem[i])] = nullptr;
            }
        }

    }

    void _push_prefix(trie_node* st_node, std::string& stem, size_t st) {
        if (st == stem.size()) {
            st_node->count += 1;
            return;
        }

        st_node->ch[get_index(stem[st])] = new trie_node;
        _push_prefix(st_node->ch[get_index(stem[st])], stem, st + 1);
    }

    void _clear(trie_node* root) {
        if (root == nullptr) {
            return;
        }

        for (size_t i = 0; i < CH_SIZE; i++) {
            if (root->ch[i] != nullptr) {
                _clear(root->ch[i]);
            }
        }

        delete root;
        root = nullptr;
    }

    void traverse(std::unordered_map<std::string, size_t>& tf_map, trie_node* root, std::string& temp) {
        for(size_t i = 0; i < CH_SIZE; i++) {
            if(root->ch[i] != nullptr) {
                temp += ('a' + i);
                traverse(tf_map, root->ch[i], temp);
            }
            temp.resize(temp.size()-1);
            tf_map.insert(std::make_pair(temp, this->root->ch[i]->count));
        }
    }
public:
    trie() { root = new trie_node; }

    trie(std::vector<std::string>& list) {
        root = new trie_node;

        for (auto s : list)
            insert(s);
    }

    ~trie() { clear(); }

    trie_node* find(std::string& stem) { return _find(root, stem, 0); }

    void insert(std::string& stem) { _insert(root, stem, 0); }

    void erase(std::string& stem) { _erase(root, stem, 0); }

    bool empty() { return ch_count == 0; }

    bool contains(std::string& stem) {
        trie_node* node = find(stem);

        if (node == nullptr) {
            return false;
        }
        else {
            return node->count != 0;
        }
    }

    size_t count(std::string& stem) {
        trie_node* node = find(stem);

        if (node == nullptr) {
            return 0;
        }
        else {
            return node->count;
        }
    }

    size_t count(std::vector<std::string>& stems) {
        size_t ans = 0;
        trie_node* temp = nullptr;

        for (size_t i = 0; i < stems.size(); i++) {
            temp = this->find(stems[i]);
            if (temp != nullptr) {
                ans += temp->count;
            }
        }

        return ans;
    }

    std::unordered_map<std::string, size_t> get_tf_map() {
        std::unordered_map<std::string, size_t> tf_map;
        std::string temp = "";
        traverse(tf_map, root, temp);
        return tf_map;
    }

    void clear() {
        _clear(root);
        root = nullptr;
    }
};
