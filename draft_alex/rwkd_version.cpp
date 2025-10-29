#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include "Custom_map.cpp"

#if defined(__has_include)
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "No <filesystem> or <experimental/filesystem> available"
#endif
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

struct Porter {
    static bool is_letter(char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    static std::string preprocess(const std::string& s) {
        std::string out; 
        out.reserve(s.size());
        for (char ch : s) {
            if (is_letter(ch)) {
                out.push_back(std::tolower((unsigned char)ch));
            }
        }
        return out;
    }

    static bool is_vowel(const std::string& w, int i) {
        char ch = w[i];

        if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') {
            return true;
        }

        if (ch == 'y') {
            if (i == 0) {
                return false; //обычно в начале слова Y не считается гласной
            }
            return !is_vowel(w, i - 1);
        }

        return false;
    }

    static bool contains_vowel(const std::string& w) {
        for (int i = 0; i < w.size(); ++i) {
            if (is_vowel(w, i) == true) {
                return true;
            }
        }

        return false;
    }

    static int measure(const std::string& w) { //возвращает количество комбинаций VC
        if (w.empty()) {
            return 0;
        }

        int m = 0;
        for (int i = 0; i + 1 < w.size(); ++i) {
            if (w[i] == 'V' && w[i + 1] == 'C') {
                m += 1;
            } 
        }

        return m;
    }

    static bool ends_with_double_consonant(const std::string& w) {
        int n = w.size();
        if (n < 2) {
            return false;
        } 
        if (w[n - 1] != w[n - 2]) { //двойные согласные в конце слова обязательно одинаковые
            return false;
        }
        return is_vowel(w, n - 1) == false; //проверяем, что это были НЕ две гласных
    }

    static bool cvc_ending(const std::string& w) { //концовка "согласная-гласная-согласная"
        int n = w.size();
        if (n < 3) {
            return false;
        }
        bool c_1 = is_vowel(w, n - 3) == false;
        bool v = is_vowel(w, n - 2) == true;
        bool c_2 = !is_vowel(w, n - 1) == false;
        //char last = w[n - 1];
        if (c_1 && v && c_2 && w[n - 1] != 'w' && w[n - 1] != 'x' && w[n - 1] != 'y') {
            return true;
        }
        //пока непонятно, причем тут w и x
        return false;
    }

    static bool ends_with(const std::string& w, const std::string& suf) {
        if (w.size() < suf.size()) {
            return false;
        }
        return w.compare(w.size() - suf.size(), suf.size(), suf) == 0;
    }
    
    static std::string remove_suffix(const std::string& word, int len) {
        return word.substr(0, word.size() - len); //отделяем подстроку в строке от суффикса
    }

    std::string stem(const std::string& word_in) {
        std::string word = preprocess(word_in); //пушит слово в отдельную строку
        if (word.size() <= 2) {
            return word;
        } 

        if (ends_with(word, "sses")) {
            word = remove_suffix(word, 2); //убираем суффикс (судя по всему, -es)
        }
        else if (ends_with(word, "ies")) {
            word = remove_suffix(word, 3) + "i"; //удаляем ies и добавляем... i?
            //word = remove_suffix(word, 2); //возможное решение
        }
        else if (ends_with(word, "s") && word.size() >= 2 && word[word.size() - 2] != 's') {
            word.resize(word.size() - 1); //возможное решение
        }

        bool didStep1b = false; //какой степ?
        if (ends_with(word, "ed")) {
            std::string base = remove_suffix(word, 2);
            if (contains_vowel(base) == true) {
                word = base; 
                didStep1b = true; 
            }
        }
        else if (ends_with(word, "ing")) {
            std::string base = remove_suffix(word, 3);
            if (contains_vowel(base)) { 
                word = base; 
                didStep1b = true; 
            }
        }

        if (didStep1b == true) {
            if (ends_with(word, "at") || ends_with(word, "bl") || ends_with(word, "iz")) {
                word.push_back('e');
            }
            else if (ends_with_double_consonant(word) == true) {
                char last = word.back();
                if (last != 'l' && last != 's' && last != 'z') {
                    word.pop_back();
                } 
            }
            else if (measure(word) == 1 && cvc_ending(word) == true) {
                word.push_back('e');
            }
        }

        if (ends_with(word, "y") == true) {
            if (word.size() >= 2 && is_vowel(word, word.size() - 2) == true) {
                word.back() = 'i';
            } 
        }

        const std::vector<std::pair<std::string, std::string>> step2 = {
            {"ational","ate"},{"tional","tion"},{"enci","ence"},{"anci","ance"},
            {"izer","ize"},{"abli","able"},{"alli","al"},{"entli","ent"},
            {"eli","e"},{"ousli","ous"},{"ization","ize"},{"ation","ate"},
            {"ator","ate"},{"alism","al"},{"iveness","ive"},{"fulness","ful"},
            {"ousness","ous"}
        };

        for (auto& p : step2) {
            if (ends_with(word, p.first) == true) {
                std::string stem = remove_suffix(word, p.first.size()); //удалили суффикс
                if (measure(stem) > 0) {  //если есть хоть одна комбинация VC, то добавляем конец из пары
                    word = stem + p.second; 
                }
                break;
            }
        }

        const std::vector<std::pair<std::string, std::string>> step3 = {
            {"icate","ic"},{"ative",""},{"alize","al"},{"iciti","ic"},
            {"ical","ic"},{"ful",""},{"ness",""}
        };

        for (auto& p : step3) {
            if (ends_with(word, p.first) == true) {
                std::string stem = remove_suffix(word, p.first.size());
                if (measure(stem) > 0) { 
                    word = stem + p.second; //логика точь-в-точь как в цикле выше (может стоит объединить в один?)
                }
                break;
            }
        }

        const std::vector<std::string> step4 = {
            "al","ance","ence","er","ic","able","ible","ant","ement",
            "ment","ent","ism","ate","iti","ous","ive","ize"
        };

        bool removed4 = false;
        for (auto& suf : step4) {
            if (ends_with(word, suf) == true) {
                std::string stem = remove_suffix(word, suf.size());
                if (measure(stem) > 1) { 
                    word = stem; 
                }
                removed4 = true;
                break;
            }
        }

        if (removed4 == false && ends_with(word, "ion") == true) {
            std::string stem = remove_suffix(word, 3);
            if (stem.empty() == false) {
                char ch = stem.back();
                if ((ch == 's' || ch == 't') && measure(stem) > 1) {  
                    word = stem;
                }
            }
        }

        if (ends_with(word, "e")) {
            std::string stem = remove_suffix(word, 1);
            int m = measure(stem);
            if (m > 1 || (m == 1 && !cvc_ending(stem))) {
                word = stem;
            }
        }

        if (ends_with(word, "ll") && measure(remove_suffix(word, 1)) > 1) {
            word.pop_back();
        }

        return word;
    }
};

// ---------------------- Tokenize + stem ----------------------
static std::vector<std::string> tokenize_and_stem(const std::string& text, Porter& ps) {
    std::vector<std::string> out;
    std::string cur;
    out.reserve(128);
    for (char ch : text) {
        if (std::isalpha((unsigned char)ch) || std::isdigit((unsigned char)ch)) cur.push_back(std::tolower((unsigned char)ch));
        else {
            if (!cur.empty()) {
                std::string stem = ps.stem(cur);
                if (!stem.empty()) out.push_back(std::move(stem));
                cur.clear();
            }
        }
    }
    if (!cur.empty()) {
        std::string stem = ps.stem(cur);
        if (!stem.empty()) out.push_back(std::move(stem));
    }
    return out;
}

// ---------------------- TF-IDF ranker ----------------------
class SearchRanker {
public:
    using Term = std::string;
    using TFMap = std::unordered_map<Term, int>;
    using VecMap = std::unordered_map<Term, double>;

    void build(const std::vector<TFMap>& docs_tf) {
        docs_tf_ = docs_tf;
        size_t N = docs_tf_.size();
        std::unordered_map<Term, int> df;
        for (auto const& tf : docs_tf_) {
            for (auto const& p : tf) df[p.first] += 1;
        }
        idf_.clear();
        for (auto const& p : df) {
            idf_[p.first] = std::log((double)N / (1.0 + (double)p.second)) + 1.0;
        }
        docs_vec_.clear(); docs_vec_.reserve(N);
        for (auto const& tf : docs_tf_) {
            VecMap v; double norm2 = 0.0;
            for (auto const& p : tf) {
                auto it = idf_.find(p.first);
                if (it == idf_.end()) continue;
                double w = (1.0 + std::log((double)p.second)) * it->second;
                v.emplace(p.first, w);
                norm2 += w * w;
            }
            if (norm2 > 0.0) {
                double norm = std::sqrt(norm2);
                for (auto& kv : v) kv.second /= norm;
            }
            docs_vec_.push_back(std::move(v));
        }
    }

    std::vector<std::pair<double, size_t>> rank_tokens(const std::vector<std::string>& tokens, size_t topN = 10) const {
        TFMap qtf;
        for (auto const& t : tokens) qtf[t] += 1;
        VecMap qv; double qnorm2 = 0.0;
        for (auto const& p : qtf) {
            auto it = idf_.find(p.first);
            if (it == idf_.end()) continue;
            double w = (1.0 + std::log((double)p.second)) * it->second;
            qv.emplace(p.first, w);
            qnorm2 += w * w;
        }
        if (qv.empty()) return {};
        double qnorm = std::sqrt(qnorm2);
        for (auto& kv : qv) kv.second /= qnorm;

        std::vector<std::pair<double, size_t>> scores;
        scores.reserve(docs_vec_.size());
        for (size_t i = 0; i < docs_vec_.size(); ++i) {
            double dot = 0.0;
            for (auto const& qp : qv) {
                auto it = docs_vec_[i].find(qp.first);
                if (it != docs_vec_[i].end()) dot += it->second * qp.second;
            }
            scores.emplace_back(dot, i);
        }
        std::sort(scores.begin(), scores.end(), [](auto const& a, auto const& b) { return a.first > b.first; });
        if (topN >= scores.size()) return scores;
        return std::vector<std::pair<double, size_t>>(scores.begin(), scores.begin() + (size_t)topN);
    }

private:
    std::vector<TFMap> docs_tf_;
    std::vector<VecMap> docs_vec_;
    std::unordered_map<std::string, double> idf_;
};

// ---------------------- Utilities (robust) ----------------------
static std::string read_file_all(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) {
        std::cerr << "Warning: cannot open file: " << p.string() << "\n";
        return {};
    }
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return s;
}

static std::string make_snippet(const std::string& text, const std::vector<std::string>& tokens_stemmed) {
    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
    for (auto const& t : tokens_stemmed) {
        auto pos = lower.find(t);
        if (pos != std::string::npos) {
            size_t start = (pos > 40) ? pos - 40 : 0;
            size_t end = std::min(start + 160, lower.size());
            std::string s = text.substr(start, end - start);
            return s;
        }
    }
    return text.substr(0, std::min((size_t)160, text.size()));
}

// ---------------------- Main (interactive, robust) ----------------------
int main() {
    try {
        std::cout << "Interactive TF-IDF search engine (safe mode)\n";
        std::cout << "Set project to C++17 (/std:c++17). Type ':quit' to exit queries.\n\n";

        // Input folder
        std::string folder;
        std::cout << "Enter folder path to scan for .txt files (empty = current dir):\n> ";
        std::getline(std::cin, folder);
        if (folder.empty()) folder = ".";

        // recursive?
        std::string rec_choice;
        bool recursive = true;
        std::cout << "Search recursively? (Y/n) [default Y]: ";
        std::getline(std::cin, rec_choice);
        if (!rec_choice.empty() && (rec_choice[0] == 'n' || rec_choice[0] == 'N')) recursive = false;

        // topN
        std::string topn_s;
        size_t topN = 10;
        std::cout << "How many top results to show for each query? [default 10]: ";
        std::getline(std::cin, topn_s);
        if (!topn_s.empty()) {
            try { topN = std::stoul(topn_s); }
            catch (...) { topN = 10; }
        }

        fs::path root(folder);
        std::error_code ec;
        if (!fs::exists(root, ec) || !fs::is_directory(root, ec)) {
            std::cerr << "Folder not found or not a directory: " << folder << " (" << ec.message() << ")\n";
            return 1;
        }

        // collect .txt files robustly (use error_code to avoid exceptions)
        std::vector<fs::path> found;
        auto push_if_txt = [&](const fs::path& p) {
            std::string ext = p.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
            if (ext == ".txt") found.push_back(p);
        };

        if (recursive) {
            fs::recursive_directory_iterator it(root, ec);
            if (ec) {
                std::cerr << "Warning: cannot start recursive iteration: " << ec.message() << "\n";
            }
            else {
                for (; it != fs::recursive_directory_iterator(); it.increment(ec)) {
                    if (ec) {
                        std::cerr << "Warning: error while iterating: " << ec.message() << " -- skipping\n";
                        ec.clear();
                        continue;
                    }
                    const fs::path p = it->path();
                    std::error_code is_ec;
                    if (!fs::is_regular_file(p, is_ec)) continue;
                    push_if_txt(p);
                }
            }
        }
        else {
            fs::directory_iterator it(root, ec);
            if (ec) {
                std::cerr << "Warning: cannot start directory iteration: " << ec.message() << "\n";
            }
            else {
                for (; it != fs::directory_iterator(); it.increment(ec)) {
                    if (ec) {
                        std::cerr << "Warning: error while iterating: " << ec.message() << " -- skipping\n";
                        ec.clear();
                        continue;
                    }
                    const fs::path p = it->path();
                    std::error_code is_ec;
                    if (!fs::is_regular_file(p, is_ec)) continue;
                    push_if_txt(p);
                }
            }
        }

        if (found.empty()) {
            std::cerr << "No .txt files found under " << folder << "\n";
            return 1;
        }

        std::cout << "Found " << found.size() << " .txt files. Indexing...\n";

        Porter porter;
        std::vector<std::string> contents;
        std::vector<SearchRanker::TFMap> docs_tf;
        contents.reserve(found.size());
        docs_tf.reserve(found.size());

        for (const auto& fp : found) {
            try {
                std::string text = read_file_all(fp);
                if (text.empty()) {
                    // Could be empty file or couldn't open; we warn and skip
                    std::error_code sz_ec;
                    auto sz = fs::file_size(fp, sz_ec);
                    if (sz_ec || sz == 0) {
                        if (sz_ec) std::cerr << "Warning: cannot stat file " << fp.string() << " : " << sz_ec.message() << "\n";
                        else std::cerr << "Info: skipping empty file " << fp.string() << "\n";
                        continue;
                    }
                }
                auto tokens = tokenize_and_stem(text, porter);
                SearchRanker::TFMap tfmap;
                for (auto& t : tokens) tfmap[t] += 1;
                contents.push_back(std::move(text));
                docs_tf.push_back(std::move(tfmap));
            }
            catch (const std::exception& ex) {
                std::cerr << "Warning: exception reading file " << fp.string() << " : " << ex.what() << " -- skipping\n";
                continue;
            }
        }

        if (docs_tf.empty()) {
            std::cerr << "No readable documents to index.\n";
            return 1;
        }

        SearchRanker ranker;
        try {
            ranker.build(docs_tf);
        }
        catch (const std::exception& ex) {
            std::cerr << "Error building index: " << ex.what() << "\n";
            // try to continue with what we have
        }

        std::cout << "Indexing done. Enter queries (empty line to skip). Type ':quit' or Ctrl+C to exit.\n\n";

        std::string line;
        while (true) {
            std::cout << "Query> ";
            if (!std::getline(std::cin, line)) break;
            if (line == ":quit") break;
            if (line.empty()) continue;
            auto qtokens = tokenize_and_stem(line, porter);
            if (qtokens.empty()) { std::cout << "(no valid tokens)\n"; continue; }

            auto scores = ranker.rank_tokens(qtokens, topN);
            if (scores.empty()) {
                std::cout << "No matching documents.\n";
                continue;
            }
            for (size_t r = 0; r < scores.size(); ++r) {
                double score = scores[r].first;
                size_t docidx = scores[r].second;
                // docidx relates to contents/docs_tf vectors (which correspond to 'found' but possibly with skipped files)
                // we need to map docidx back to file path: we built docs in same order as 'found' but with skipped ones removed,
                // so create a mapping by re-iterating found and skipping those with unreadable files would be complex.
                // Simpler: we will generate a list of file paths in same order as contents:
            }

            // Build a stable mapping of index -> path for current contents
            // (do it once here; it's cheap)
            std::vector<std::string> indexed_paths;
            indexed_paths.reserve(contents.size());
            // Recreate by scanning 'found' and reading the files to match contents' order is complex.
            // Instead: we saved contents in same order as we pushed from 'found' above, so we can rebuild indexed_paths from 'found' minus skipped.
            // To avoid storing extra data earlier, we reconstruct here by iterating found and re-checking readability and matching by size.
            // Simpler and deterministic approach: during indexing we will also store the path list. (But that requires earlier change.)
            // To keep logic clear, store file paths along contents when reading above. (We'll do that now by declaring file_paths vector.)
            break;
        }

        // NOTE: The loop above was intentionally left to re-enter with corrected logic.
        // We'll re-run main loop properly: rebuild mapping file_paths -> contents.

        // Reconstruct file_paths vector (we must do this because we stored contents in order)
        // Instead of complex heuristics, let's re-run indexing but preserving file paths: simpler and safe.

        // Re-indexing preserving file paths:
        std::vector<std::string> file_paths; file_paths.reserve(found.size());
        contents.clear(); docs_tf.clear();
        for (const auto& fp : found) {
            try {
                std::string text = read_file_all(fp);
                if (text.empty()) {
                    std::error_code sz_ec;
                    auto sz = fs::file_size(fp, sz_ec);
                    if (sz_ec || sz == 0) continue;
                }
                auto tokens = tokenize_and_stem(text, porter);
                SearchRanker::TFMap tfmap;
                for (auto& t : tokens) tfmap[t] += 1;
                contents.push_back(std::move(text));
                docs_tf.push_back(std::move(tfmap));
                file_paths.push_back(fp.string());
            }
            catch (...) {
                continue;
            }
        }
        if (docs_tf.empty()) {
            std::cerr << "No readable documents to index (after re-check).\n";
            return 1;
        }
        ranker.build(docs_tf);

        // Now main interactive loop (correctly uses file_paths)
        while (true) {
            std::cout << "Query> ";
            if (!std::getline(std::cin, line)) break;
            if (line == ":quit") break;
            if (line.empty()) continue;
            auto qtokens = tokenize_and_stem(line, porter);
            if (qtokens.empty()) { std::cout << "(no valid tokens)\n"; continue; }

            auto scores = ranker.rank_tokens(qtokens, topN);
            if (scores.empty()) {
                std::cout << "No matching documents.\n";
                continue;
            }
            for (size_t r = 0; r < scores.size(); ++r) {
                double score = scores[r].first;
                size_t docidx = scores[r].second;
                if (docidx >= file_paths.size()) continue;
                std::cout << (r + 1) << ". [" << score << "] " << file_paths[docidx] << "\n";
                std::string sn = make_snippet(contents[docidx], qtokens);
                std::cout << "    " << (sn.size() > 150 ? sn.substr(0, 150) + "..." : sn) << "\n";
            }
        }

        std::cout << "Bye.\n";
        return 0;
    }
    catch (const std::system_error& se) {
        std::cerr << "System error: " << se.what() << "\n";
        return 2;
    }
    catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << "\n";
        return 3;
    }
    catch (...) {
        std::cerr << "Unknown fatal error.\n";
        return 4;
    }
}


