#include "stemmer.cpp"
#include "trie.cpp"

std::vector<std::string> tokenize(const std::string& text) {
	std::vector<std::string> tokens;
	std::string temp;
	tokens.reserve(128);

	for (char ch : text) {
		if (std::isalpha((unsigned char)ch) || std::isdigit((unsigned char)ch)) {
			temp.push_back(std::tolower((unsigned char)ch));
		}
		else {
			if (!temp.empty()) {
				tokens.push_back(std::move(temp));
				temp.clear();
				temp.reserve(16);
			}
		}
	}
	if (!temp.empty())
		tokens.push_back(std::move(temp));
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

std::vector<std::string> get_tokens(const std::string& text) {
	auto toks = tokenize(text);
	return stem_tokens(toks);
}
