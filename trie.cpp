#include <string>
#include <vector>

#define TRIE
#define UNIT_TESTS

struct trie_node {
    std::string prefix;
    std::vector<trie_node*> ch;

    trie_node() {
        prefix = "";
        ch = {};
    }

    trie_node(trie_node& par, char& sym) {
        prefix = par.prefix + sym;
        ch = {};
    }

    trie_node(std::string& stem) {
        prefix = stem;
        ch = {};
    }
};

class trie {
private:
    trie_node* root;

    void _insert(trie_node* root, std::string& stem, size_t i) {
        if(root == nullptr) {
            root = new trie_node(stem);
            return;
        }

        size_t j = -1;
        for(auto it = root->ch.begin(); it != root->ch.end(); it++) {
            
        }
    }

    void _clear(trie_node* root) {
        for(size_t i = 0; i < root->ch.size(); i++) 
            _clear(root->ch[i]);
        free(root);
    }
public:
    trie() {}

    ~trie() { _clear(root); } 

    void insert(std::string& stem) { _insert(root, stem, 0); }

    void erase(std::string& stem) {}

    void find(std::string& stem) {}

    void clear() { 
        _clear(root); 
        root = nullptr;
    }
};

#ifdef UNIT_TESTS

int main(void) 
{

    return 0;
}

#endif