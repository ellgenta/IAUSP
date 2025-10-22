#include <iostream>
#include "trie.cpp"
#include <unordered_map> 

using doc_name = std::string;
using stemmed_doc = trie;

class map {
private:
    std::unordered_map<doc_name, stemmed_doc*> _map;
public:
    map() = default;

    ~map() {
        for (auto _it : _map) {
            _it.second->clear();
            delete _it.second;
            _it.second = nullptr;
        }
    }

    void insert(doc_name& name, std::vector<std::string>& stems) {
        stemmed_doc* _d = new stemmed_doc(stems);
        _map.insert({ name, _d });
    }

    size_t get_index(std::string& stem) {
        size_t indx = 0;

        for (auto _p : _map)
            indx += _p.second->count(stem);

        return indx;
    }

    size_t get_index(std::vector<std::string>& query) {
        size_t indx = 0;

        for (auto _p : _map) {
            indx += _p.second->count(query);
        }

        return indx;
    }

    std::vector<doc_name> get_names(std::string& stem) {
        std::vector<doc_name> keys;

        for (auto _p : _map) {
            if (_p.second->contains(stem) == true)
                keys.push_back(_p.first);
        }

        return keys;
    }
};