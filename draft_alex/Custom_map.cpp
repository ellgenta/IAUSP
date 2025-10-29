#include <iostream>
#include "Trie.cpp"
#include <unordered_map> 

using doc_name = std::string;
using stemmed_doc = trie;

class map {
private:
    std::unordered_map<doc_name, stemmed_doc*> _map;
public:
    map() = default;

    ~map() {
        for(auto _it : _map) {
            _it.second->clear();
            delete _it.second;
            _it.second = nullptr;
        }
    }

    void insert(doc_name& name, std::vector<std::string>& stems) {
        stemmed_doc* _d = new stemmed_doc(stems);
        _map.insert({name, _d});
    }

    size_t get_index(std::string& stem) {
        size_t indx = 0;

        for(auto _p : _map) 
            indx += _p.second->count(stem);

        return indx;
    }

    size_t get_index(std::vector<std::string>& query) {
        size_t indx = 0;

        for(auto _p : _map) {
            indx += _p.second->count(query);
        } 

        return indx;
    }

    std::vector<doc_name> get_names(std::string& stem) {
        std::vector<doc_name> keys;

        for(auto _p : _map) {
            if(_p.second->contains(stem) == true)
                keys.push_back(_p.first);
        }

        return keys;
    }
};

map test_map;

#define TESTS
#undef TESTS

#ifdef TESTS

#include <cassert>

int main(void) 
{
    std::vector<std::vector<std::string>> docs_content {
        {"42","is","the","answer"},
        {"Jacque","Fresco","was","a","genius"},
        {"Jacque","Fresco","was","born","in","1000","B.C."},
        {"Let","it","be","said","mr.","Fresco"}
    };
    
    std::vector<std::string> doc_names {
        "ans",
        "genius",
        "bio",
        "meme"
    };

    for(size_t i = 0; i < 4; i++) 
        test_map.insert(doc_names[i], docs_content[i]);

    std::string entry_Jacque = "Jacque";
    std::string entry_genius = "genius";
    std::string entry_Moldova = "Moldova";
    
    assert(test_map.get_index(entry_Jacque) == 2);
    assert(test_map.get_index(entry_genius) == 1);
    assert(test_map.get_index(entry_Moldova) == 0);

    std::vector<doc_name> keys = test_map.get_names(entry_Jacque);
    
    assert(keys.size() == 2);
    assert(keys[0] == "bio" || keys[1] == "bio" && keys[0] == "genius" || keys[1] == "genius");
 
    return 0;
}

#endif
