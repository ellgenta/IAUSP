#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

class SearchRanker {
public:
	using Term = std::string;
	using TermFrequencyMap = std::unordered_map<Term, int>;
	using WeightVector = std::unordered_map<Term, double>;
	using ScorePair = std::pair<double, size_t>;

	void build(const std::vector<TermFrequencyMap>& documents_term_frequencies) {
		documents_term_frequencies_ = documents_term_frequencies;

		if (documents_term_frequencies_.empty()) {
			inverse_document_frequencies_.clear();
			document_vectors_.clear();
			return;
		}

		calculate_inverse_document_frequencies();
		build_document_vectors();
	}

	std::vector<ScorePair> rank_tokens(const std::vector<std::string>& tokens, size_t top_results_count = 10) const {
		if (tokens.empty() || document_vectors_.empty()) {
			return {};
		}

		WeightVector query_vector = build_query_vector(tokens);
		if (query_vector.empty()) {
			return {};
		}

		std::vector<ScorePair> document_scores =
			calculate_document_scores(query_vector);

		return get_top_results(document_scores, top_results_count);
	}

private:
	std::vector<TermFrequencyMap> documents_term_frequencies_;
	std::vector<WeightVector> document_vectors_;
	std::unordered_map<Term, double> inverse_document_frequencies_;

	void calculate_inverse_document_frequencies() {
		std::unordered_map<Term, int> document_frequency;

		for (const auto& term_frequencies : documents_term_frequencies_) {
			for (const auto& kv : term_frequencies) {
				document_frequency[kv.first] += 1;
			}
		}

		inverse_document_frequencies_.clear();
		inverse_document_frequencies_.reserve(document_frequency.size());
		const double total_documents = static_cast<double>(documents_term_frequencies_.size());

		for (const auto& kv : document_frequency) {
			inverse_document_frequencies_[kv.first] =
				std::log(total_documents / (1.0 + static_cast<double>(kv.second))) + 1.0;
		}
	}

	void build_document_vectors() {
		document_vectors_.clear();
		document_vectors_.reserve(documents_term_frequencies_.size());

		for (const auto& term_frequencies : documents_term_frequencies_) {
			WeightVector doc_vec = build_document_vector(term_frequencies);
			document_vectors_.push_back(std::move(doc_vec));
		}
	}

	WeightVector build_document_vector(const TermFrequencyMap& term_frequencies) const {
		WeightVector document_vector;
		document_vector.reserve(term_frequencies.size());
		double squared_norm = 0.0;

		for (const auto& kv : term_frequencies) {
			const Term& term = kv.first;
			int frequency = kv.second;
			auto it = inverse_document_frequencies_.find(term);
			if (it == inverse_document_frequencies_.end()) continue;

			const double term_weight = (1.0 + std::log(static_cast<double>(frequency))) * it->second;
			document_vector.emplace(term, term_weight);
			squared_norm += term_weight * term_weight;
		}

		if (squared_norm > 0.0) {
			normalize_vector(document_vector, squared_norm);
		}

		return document_vector;
	}

	WeightVector build_query_vector(const std::vector<std::string>& tokens) const {
		TermFrequencyMap query_term_frequencies;
		for (const auto& token : tokens) {
			query_term_frequencies[token] += 1;
		}

		return build_and_normalize_vector(query_term_frequencies);
	}

	WeightVector build_and_normalize_vector(const TermFrequencyMap& frequencies) const {
		WeightVector vector;
		double squared_norm = 0.0;

		for (const auto& [term, freq] : frequencies) {
			auto it = inverse_document_frequencies_.find(term);
			if (it == inverse_document_frequencies_.end()) continue;

			double weight = (1.0 + std::log(static_cast<double>(freq))) * it->second;
			vector[term] = weight;
			squared_norm += weight * weight;
		}

		if (squared_norm > 0.0) {
			normalize_vector(vector, squared_norm);
		}

		return vector;
	}

	void normalize_vector(WeightVector& vector) const {
		double squared_norm = 0.0;
		for (const auto& kv : vector) squared_norm += kv.second * kv.second;
		normalize_vector(vector, squared_norm);
	}

	void normalize_vector(WeightVector& vector, double squared_norm) const {
		if (squared_norm <= 0.0) return;
		const double norm = std::sqrt(squared_norm);
		for (auto& kv : vector) kv.second /= norm;
	}

	std::vector<ScorePair> calculate_document_scores(
		const WeightVector& query_vector
	) const {
		std::vector<ScorePair> scores;
		scores.reserve(document_vectors_.size());

		for (size_t idx = 0; idx < document_vectors_.size(); ++idx) {
			double similarity_score = calculate_cosine_similarity(
				query_vector,
				document_vectors_[idx]
			);
			scores.emplace_back(similarity_score, idx);
		}

		return scores;
	}

	double calculate_cosine_similarity(
		const WeightVector& query_vector,
		const WeightVector& document_vector
	) const {
		double dot_product = 0.0;
		for (const auto& kv : query_vector) {
			auto it = document_vector.find(kv.first);
			if (it != document_vector.end()) {
				dot_product += it->second * kv.second;
			}
		}
		return dot_product;
	}

	std::vector<ScorePair> get_top_results(
		std::vector<ScorePair>& scores,
		size_t top_results_count
	) const {
		if (scores.empty()) return {};

		if (top_results_count >= scores.size()) {
			std::sort(scores.begin(), scores.end(), [](const ScorePair& a, const ScorePair& b) {
				return a.first > b.first;
			});
			return scores;
		}

		// Для эффективности: частичная сортировка до top_N
		std::partial_sort(scores.begin(), scores.begin() + top_results_count, scores.end(),
			[](const ScorePair& a, const ScorePair& b) { return a.first > b.first; });

		return std::vector<ScorePair>(scores.begin(), scores.begin() + top_results_count);
	}
};
struct Porter {
	static bool is_letter(char ch) {
		return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
	}
	static std::string get_only_letters(const std::string& s) {
		std::string out; 
		out.reserve(s.size());
		for (char ch : s) 
			if (is_letter(ch)) 
				out.push_back(std::tolower((unsigned char)ch));
		return out;
	}
	static bool is_vowel(const std::string& word, int i) {
		char ch = word[i];
		if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') return true;
		if (ch == 'y') {
			if (i == 0) return false;
			return !is_vowel(word, i - 1);
		}
		return false;
	}
	static bool contains_vowel(const std::string& word) {
		for (int i = 0; i < (int)word.size(); ++i) 
			if (is_vowel(word, i)) 
				return true;
		return false;
	}
	static int measure(const std::string& word) {
		int vc_transitions = 0;
		bool previous_was_vowel = false;
		for (int i = 0; i < (int)word.size(); ++i) {
			bool current_is_vowel = is_vowel(word, i);
			if (previous_was_vowel && !current_is_vowel) 
				++vc_transitions;
			previous_was_vowel = current_is_vowel;
		}
		return vc_transitions;
	}
	static bool ends_with_double_consonant(const std::string& word) {
		int word_size = (int)word.size();
		if (word_size < 2) return false;
		char current = word[word_size - 1], previous = word[word_size - 2];
		if (current != previous) return false;
		return !is_vowel(word, word_size - 1);
	}
	static bool cvc_ending(const std::string& word) {
		int word_size = (int)word.size();
		if (word_size < 3) return false;
		bool c1 = !is_vowel(word, word_size - 3);
		bool v = is_vowel(word, word_size - 2);
		bool c2 = !is_vowel(word, word_size - 1);
		char last = word[word_size - 1];
		if (c1 && v && c2 && last != 'w' && last != 'x' && last != 'y') return true;
		return false;
	}
	static bool ends_with(const std::string& word, const std::string& suffix) {
		if ((int)word.size() < (int)suffix.size()) return false;
		return word.compare(word.size() - suffix.size(), suffix.size(), suffix) == 0;
	}
	static std::string remove_suffix(const std::string& word, int len) {
		return word.substr(0, word.size() - len);
	}
	static std::string stem(const std::string& word_in) {
		std::string word = get_only_letters(word_in);
		if ((int)word.size() <= 2) return word;

		if (ends_with(word, "sses")) 
			word = remove_suffix(word, 2);
		else if (ends_with(word, "ies")) 
			word = remove_suffix(word, 3) + "i";
		else if (ends_with(word, "s")) 
			if (word.size() >= 2 && word[word.size() - 2] != 's') 
				word = remove_suffix(word, 1);

		bool removed_ed_or_ing = false;
		if (ends_with(word, "ed")) {
			std::string base = remove_suffix(word, 2);
			if (contains_vowel(base)) { word = base; removed_ed_or_ing = true; }
		}
		else if (ends_with(word, "ing")) {
			std::string base = remove_suffix(word, 3);
			if (contains_vowel(base)) { word = base; removed_ed_or_ing = true; }
		}
		if (removed_ed_or_ing) {
			if (ends_with(word, "at") || ends_with(word, "bl") || ends_with(word, "iz")) word.push_back('e');
			else if (ends_with_double_consonant(word)) {
				char last = word.back();
				if (last != 'l' && last != 's' && last != 'z') word.pop_back();
			}
			else if (measure(word) == 1 && cvc_ending(word)) word.push_back('e');
		}
		if (ends_with(word, "y")) {
			if (word.size() >= 2 && is_vowel(word, (int)word.size() - 2)) word.back() = 'i';
		}
		const std::vector<std::pair<std::string, std::string>> suffix_replacements_1 = {
			{"ational","ate"},{"tional","tion"},{"enci","ence"},{"anci","ance"},
			{"izer","ize"},{"abli","able"},{"alli","al"},{"entli","ent"},
			{"eli","e"},{"ousli","ous"},{"ization","ize"},{"ation","ate"},
			{"ator","ate"},{"alism","al"},{"iveness","ive"},{"fulness","ful"},
			{"ousness","ous"}
		};
		for (auto& p : suffix_replacements_1) {
			if (ends_with(word, p.first)) {
				std::string stem = remove_suffix(word, (int)p.first.size());
				if (measure(stem) > 0) { word = stem + p.second; }
				break;
			}
		}
		const std::vector<std::pair<std::string, std::string>> suffix_replacements_2 = {
			{"icate","ic"},{"ative",""},{"alize","al"},{"iciti","ic"},
			{"ical","ic"},{"ful",""},{"ness",""}
		};
		for (auto& p : suffix_replacements_2) {
			if (ends_with(word, p.first)) {
				std::string stem = remove_suffix(word, (int)p.first.size());
				if (measure(stem) > 0) { word = stem + p.second; }
				break;
			}
		}
		const std::vector<std::string> step4 = {
			"al","ance","ence","er","ic","able","ible","ant","ement",
			"ment","ent","ism","ate","iti","ous","ive","ize"
		};
		bool removed4 = false;
		for (auto& suf : step4) {
			if (ends_with(word, suf)) {
				std::string stem = remove_suffix(word, (int)suf.size());
				if (measure(stem) > 1) { word = stem; }
				removed4 = true;
				break;
			}
		}
		if (!removed4 && ends_with(word, "ion")) {
			std::string stem = remove_suffix(word, 3);
			if (!stem.empty()) {
				char ch = stem.back();
				if ((ch == 's' || ch == 't') && measure(stem) > 1) word = stem;
			}
		}
		if (ends_with(word, "e")) {
			std::string stem = remove_suffix(word, 1);
			int m = measure(stem);
			if (m > 1 || (m == 1 && !cvc_ending(stem))) word = stem;
		}
		if (ends_with(word, "ll") && measure(remove_suffix(word, 1)) > 1) word.pop_back();

		return word;
	}
};
struct SnippetGenerator {
	static constexpr size_t CONTEXT_BEFORE = 40;
	static constexpr size_t CONTEXT_AFTER = 120;
	static constexpr size_t MAX_SNIPPET_LEN = CONTEXT_BEFORE + CONTEXT_AFTER;

	static std::string make_snippet(const std::string& text,
		const std::vector<std::string>& query_tokens,
		Porter& porter) {
		if (text.empty() || query_tokens.empty()) {
			return truncate_text(text, MAX_SNIPPET_LEN);
		}

		// Создаем стеммированные версии терминов для поиска
		std::unordered_set<std::string> search_terms;
		for (const auto& token : query_tokens) {
			std::string stemmed = porter.stem(token);
			if (!stemmed.empty()) {
				search_terms.insert(stemmed);
			}
			// Также добавляем оригинальные токены на случай, если они уже стеммированные
			search_terms.insert(token);
		}

		std::string text_lower = to_lower_case(text);
		size_t best_position = find_first_match(text_lower, search_terms);

		if (best_position == std::string::npos) {
			return truncate_text(text, MAX_SNIPPET_LEN);
		}

		return extract_context_with_highlight(text, text_lower, best_position, search_terms);
	}

private:
	static std::string to_lower_case(const std::string& str) {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return result;
	}

	static size_t find_first_match(const std::string& text_lower,
		const std::unordered_set<std::string>& terms) {
		size_t best_position = std::string::npos;

		for (const auto& term : terms) {
			if (term.empty()) continue;

			size_t position = text_lower.find(term);
			if (position != std::string::npos) {
				if (best_position == std::string::npos || position < best_position) {
					best_position = position;
				}
			}
		}

		return best_position;
	}

	static std::string extract_context_with_highlight(
		const std::string& original_text,
		const std::string& text_lower,
		size_t match_position,
		const std::unordered_set<std::string>& terms) {

		// Вычисляем границы сниппета
		size_t start = (match_position > CONTEXT_BEFORE) ?
			match_position - CONTEXT_BEFORE : 0;
		size_t end = std::min(match_position + CONTEXT_AFTER, original_text.length());

		// Извлекаем сниппет
		std::string snippet = original_text.substr(start, end - start);
		std::string snippet_lower = text_lower.substr(start, end - start);

		// Выделяем термины
		std::string highlighted = highlight_terms(snippet, snippet_lower, terms);

		// Добавляем многоточия если нужно
		if (start > 0) highlighted = "..." + highlighted;
		if (end < original_text.length()) highlighted += "...";

		return highlighted;
	}

	static std::string highlight_terms(const std::string& snippet,
		const std::string& snippet_lower,
		const std::unordered_set<std::string>& terms) {
		std::string result = snippet;
		int offset = 0; // Смещение из-за вставленных символов выделения

		// Сортируем термины по длине (от длинных к коротким)
		std::vector<std::string> sorted_terms(terms.begin(), terms.end());
		std::sort(sorted_terms.begin(), sorted_terms.end(),
			[](const std::string& a, const std::string& b) {
			return a.size() > b.size();
		});

		// Ищем и выделяем все вхождения
		for (const auto& term : sorted_terms) {
			if (term.empty()) continue;

			size_t pos = 0;
			while (pos < snippet_lower.length()) {
				size_t found_pos = snippet_lower.find(term, pos);
				if (found_pos == std::string::npos) break;

				// Вычисляем позиции с учетом предыдущих вставок
				size_t adjusted_pos = found_pos + offset;

				// Вставляем выделение
				result.insert(adjusted_pos, "[");
				result.insert(adjusted_pos + term.length() + 1, "]");

				offset += 2; // Добавили 2 символа: [ и ]
				pos = found_pos + term.length();
			}
		}

		return result;
	}

	static std::string truncate_text(const std::string& text, size_t max_length) {
		if (text.length() <= max_length) {
			return text;
		}
		return text.substr(0, max_length) + "...";
	}
};

static std::vector<std::string> tokenize(const std::string& text) {
	std::vector<std::string> tokens;
	std::string cur;
	tokens.reserve(128);
	for (char ch : text) {
		if (std::isalpha((unsigned char)ch) || std::isdigit((unsigned char)ch)) {
			cur.push_back(std::tolower((unsigned char)ch));
		}
		else {
			if (!cur.empty()) {
				tokens.push_back(std::move(cur));
				cur.clear();
				cur.reserve(16);
			}
		}
	}
	if (!cur.empty()) 
		tokens.push_back(std::move(cur));
	return tokens;
}
static std::vector<std::string> stem_tokens(const std::vector<std::string>& tokens, Porter& ps) {
	std::vector<std::string> out;
	out.reserve(tokens.size());
	for (auto const& t : tokens) {
		std::string s = Porter::stem(t);
		if (!s.empty()) out.push_back(std::move(s));
	}
	return out;
}
static std::vector<std::string> tokenize_and_stem(const std::string& text, Porter& ps) {
	auto toks = tokenize(text);
	return stem_tokens(toks, ps);
}
static std::string read_file(const fs::path& p) {
	std::ifstream in(p, std::ios::binary);
	if (!in) {
		std::cerr << "Warning: cannot open file: " << p.string() << "\n";
		return {};
	}
	std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	return s;
}

int main() {
	std::string folder_path;
	std::cout << "Enter folder path to scan for .txt files (empty = current dir):\n> ";
	std::getline(std::cin, folder_path);
	if (folder_path.empty()) folder_path = ".";

	std::string results;
	size_t shown_results_count = 10;
	std::cout << "How many top results to show for each query? [default 10]: ";
	std::getline(std::cin, results);
	if (!results.empty()) {
		try { shown_results_count = std::stoul(results); }
		catch (...) { shown_results_count = 10; }
	}

	fs::path root(folder_path);
	std::error_code error;
	if (!fs::exists(root, error) || !fs::is_directory(root, error)) {
		std::cerr << "Folder not found or not a directory: " << folder_path << " (" << error.message() << ")\n";
		return 1;
	}

	std::vector<fs::path> found;
	auto push_if_txt = [&](const fs::path& p) {
		std::string ext = p.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
		if (ext == ".txt") found.push_back(p);
	};

	fs::directory_iterator iterator(root, error);
	if (error) {
		std::cerr << "Warning: cannot start directory iteration: " << error.message() << "\n";
	}
	else {
		for (; iterator != fs::directory_iterator(); iterator.increment(error)) {
			if (error) {
				std::cerr << "Warning: error while iterating: " << error.message() << " -- skipping\n";
				error.clear();
				continue;
			}
			const fs::path p = iterator->path();
			std::error_code is_ec;
			if (!fs::is_regular_file(p, is_ec)) continue;
			push_if_txt(p);
		}
	}
	if (found.empty()) {
		std::cerr << "No .txt files found under " << folder_path << "\n";
		return 1;
	}

	std::cout << "Found " << found.size() << " .txt files. Indexing...\n";

	Porter porter;
	std::vector<std::string> file_paths; 
	std::vector<std::string> contents;
	std::vector<SearchRanker::TermFrequencyMap> docs_tf;
	file_paths.reserve(found.size());
	contents.reserve(found.size());
	docs_tf.reserve(found.size());

	for (const auto& fp : found) {
		try {
			std::string text = read_file(fp);
			if (text.empty()) {
				std::error_code sz_ec;
				auto sz = fs::file_size(fp, sz_ec);
				if (sz_ec || sz == 0) {
					if (sz_ec) std::cerr << "Warning: cannot stat file " << fp.string() << " : " << sz_ec.message() << "\n";
					else std::cerr << "Info: skipping empty file " << fp.string() << "\n";
					continue;
				}
			}
			auto tokens = tokenize_and_stem(text, porter);
			SearchRanker::TermFrequencyMap tfmap;
			for (auto& t : tokens) tfmap[t] += 1;
			file_paths.push_back(fp.string());
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
	}

	std::cout << "Indexing done. Enter queries (empty line to skip).\n\n";

	std::string user_input;
	while (true) {
		std::cout << "Query> ";
		if (!std::getline(std::cin, user_input)) break;
		if (user_input.empty()) continue;
		auto qtokens = tokenize_and_stem(user_input, porter);
		if (qtokens.empty()) { std::cout << "(no valid tokens)\n"; continue; }

		auto scores = ranker.rank_tokens(qtokens, shown_results_count);
		if (scores.empty()) {
			std::cout << "No matching documents.\n";
			continue;
		}
		for (size_t r = 0; r < scores.size(); ++r) {
			double score = scores[r].first;
			size_t docidx = scores[r].second;
			if (docidx >= file_paths.size()) continue;
			std::cout << (r + 1) << ". [" << score << "] " << file_paths[docidx] << "\n";
			SnippetGenerator generator;
			std::string snippet = SnippetGenerator::make_snippet(contents[docidx], qtokens, porter);
			std::cout << "<" << (snippet.size() > 150 ? snippet.substr(0, 150) + ">" : snippet) << "\n";
		}
	}
	return 0;
}
