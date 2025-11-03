#include "stemmer.cpp"
#include "trie.cpp"

std::vector<std::string> tokenize(const std::string& text) {
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

std::vector<std::string> stem_tokens(const std::vector<std::string>& tokens) {
	std::vector<std::string> out;
	out.reserve(tokens.size());
	for (auto const& t : tokens) {
		std::string s = porter::get_stem(t);
		if (!s.empty()) out.push_back(std::move(s));
	}
	return out;
}

std::vector<std::string> tokenize_and_stem(const std::string& text) {
	auto toks = tokenize(text);
	return stem_tokens(toks);
}