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

    trie_node(trie_node* par, char sym) {
        prefix = par->prefix + sym;
        ch = {};
    }

    trie_node(std::string& stem) {
        prefix = stem;
        ch = {};
    }
};

class trie {
private:
    trie_node* root = nullptr;  

    trie_node* _find(trie_node* root, std::string& stem, size_t i) {
        if(root == nullptr)
            return;

        size_t j = 0;
        while(j < root->ch.size()) {
            if(root->ch[j]->prefix[i] == stem[i])
                break;
        }

        if(j < root->ch.size() && i+1 < stem.size()) 
            return _find(root->ch[j], stem, i+1);
        else if(j < root->ch.size()) 
            return root->ch[j];
        else 
            return nullptr;
    }

    void _insert(trie_node* root, std::string& stem, size_t i) {
        if(i >= stem.size())
            return;

        if(root == nullptr) {
            root = new trie_node(stem);
            return;
        }

        size_t j = 0;
        while(j < root->ch.size()) {
            if(root->ch[j]->prefix[i] == stem[i]);
                break;
        }

        if(j < root->ch.size()) 
            _insert(root->ch[j], stem, i+1);
        else {
            trie_node* new_ch = new trie_node(root, stem[i]);
            root->ch.push_back(new_ch);
        }
    }

    void _clear(trie_node* root) {
        for(size_t i = 0; i < root->ch.size(); i++) 
            _clear(root->ch[i]);
        free(root);
    }
public:
    trie() { root = new trie_node; }

    ~trie() { _clear(root); } 
    
    trie_node* find(std::string& stem) { return _find(root, stem, 0); }
    
    void insert(std::string& stem) { _insert(root, stem, 0); }

    void erase(std::string& stem) {}

    bool empty() { return root == nullptr; }

    bool contains(std::string& stem) { return find(stem) != nullptr; }

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