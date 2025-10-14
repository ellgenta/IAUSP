#include <string>

#define TRIE
#define UNIT_TESTS
#define CH_SIZE  256

struct trie_node {
    std::string prefix;
    trie_node* ch[CH_SIZE] = {0};

    trie_node() { prefix = ""; }

    trie_node(trie_node* par, char sym) { prefix = par->prefix + sym; }

    trie_node(std::string& stem) { prefix = stem; }
};

class trie {
private:
    trie_node* root = nullptr;
    size_t ch_count = 0;  

    trie_node* _find(trie_node* root, std::string& stem, size_t i) {
        if(root == nullptr) {
            return nullptr;
        }

        if(i >= stem.size()) {
            return root;
        }

        if(root->ch[stem[i]] != nullptr) {
            return _find(root->ch[stem[i]], stem, i+1);
        } 

        return nullptr;
    }

    void _insert(trie_node* root, std::string& stem, size_t i) {
        if(root == nullptr || i == stem.size()) {
            return;
        }

        if(root->ch[stem[i]] != nullptr) {
            _insert(root->ch[stem[i]], stem, i+1);
        } else {
            if(root == this->root) {
                ch_count += 1;
            }
            _push_prefix(root, stem, i);
        }
    }

    void _push_prefix(trie_node* st_node, std::string& stem, size_t st) {
        if(st == stem.size())
            return;
        st_node->ch[stem[st]] = new trie_node;
        _push_prefix(st_node->ch[stem[st]], stem, st+1);
    }

    void _clear(trie_node* root) {
        if(root == nullptr) {
            return;
        }

        for(size_t i = 0; i < CH_SIZE; i++) {
            if(root->ch[i] != nullptr) {
                _clear(root->ch[i]);
            }
        }

        delete root;
    }
public:
    trie() { root = new trie_node; }

    ~trie() { _clear(root); } 
    
    trie_node* find(std::string& stem) { return _find(root, stem, 0); }
    
    void insert(std::string& stem) { _insert(root, stem, 0); }

    void erase(std::string& stem) {}

    bool empty() { return ch_count == 0; }

    bool contains(std::string& stem) { return find(stem) != nullptr; }

    void clear() { 
        _clear(root); 
        root = nullptr;
    }
};

#ifdef UNIT_TESTS

#include <cassert>

int main(void) 
{
    trie T;

    assert(T.empty() == true);

    std::string _a = "cat";
    std::string _b = "car";
    std::string _c = "carnivore";
    std::string _d = "search";

    T.insert(_a);
    T.insert(_b);
    T.insert(_c);

    assert(T.empty() == false);

    assert(T.contains(_a) == true);
    assert(T.contains(_b) == true);
    assert(T.find(_c) != nullptr);
    assert(T.find(_d) == nullptr);

    T.clear();

    return 0;
}

#endif