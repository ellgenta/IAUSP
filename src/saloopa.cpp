#include <iostream>
#include <fstream>
#include <filesystem>
#include "tf-idf.cpp"

namespace fs = std::filesystem;

static std::string read_file(const fs::path& p) {
	std::ifstream in(p, std::ios::binary);
	if (!in) {
		std::cerr << "Warning: cannot open file: " << p.string() << "\n";
		return {};
	}
	std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	return s;
}

struct SnippetGenerator {
	static constexpr size_t CONTEXT_BEFORE = 40;
	static constexpr size_t CONTEXT_AFTER = 120;
	static constexpr size_t MAX_SNIPPET_LEN = CONTEXT_BEFORE + CONTEXT_AFTER;

	static std::string make_snippet(const doc_t& doc,
		const std::vector<std::string>& query_tokens) {
		const std::filesystem::path path = doc.get_path();
		std::string text = read_file(path);
		if (text.empty() || query_tokens.empty()) {
			return truncate_text(text, MAX_SNIPPET_LEN);
		}

		// Создаем стеммированные версии терминов для поиска
		std::unordered_set<std::string> search_terms;
		for (const auto& token : query_tokens) {
			std::string stemmed = porter::get_stem(token);
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
	std::vector<doc_t> docs;

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
			docs.push_back(doc_t(fp.string(), text));
		}
		catch (const std::exception& ex) {
			std::cerr << "Warning: exception reading file " << fp.string() << " : " << ex.what() << " -- skipping\n";
			continue;
		}
	}
	
	if (docs.empty()) { //why docs-tf??
		std::cerr << "No readable documents to index.\n";
		return 1;
	}

	SearchRanker ranker;
	try {
		ranker.build(docs); 
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
		auto qtokens = tokenize_and_stem(user_input);
		if (qtokens.empty()) { 
			std::cout << "(no valid tokens)\n"; 
			continue; 
		}

		auto scores = ranker.rank_tokens(qtokens, shown_results_count);
		if (scores.empty()) {
			std::cout << "No matching documents.\n";
			continue;
		}
		for (size_t r = 0; r < scores.size(); ++r) {
			double score = scores[r].first;
			size_t docidx = scores[r].second;
			if (docidx >= docs.size()) continue;
			std::cout << (r + 1) << ". [" << score << "] " << docs[docidx].get_path() << "\n";
			SnippetGenerator generator;
			std::string snippet = SnippetGenerator::make_snippet(docs[docidx], qtokens);
			std::cout << "<" << (snippet.size() > 150 ? snippet.substr(0, 150) + ">" : snippet) << "\n";
		}
	}
	return 0;
}
