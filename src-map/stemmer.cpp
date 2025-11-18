#include <string>
#include <vector>

using suff_table_t = std::vector<std::pair<std::string, std::string>>;

const suff_table_t shrink_table_1 = {
	{"ational","ate"},{"tional","tion"},{"enci","ence"},{"anci","ance"},
	{"izer","ize"},{"abli","able"},{"alli","al"},{"entli","ent"},
	{"eli","e"},{"ousli","ous"},{"ization","ize"},{"ation","ate"},
	{"ator","ate"},{"alism","al"},{"iveness","ive"},{"fulness","ful"},
	{"ousness","ous"}
};

const suff_table_t shrink_table_2 = {
	{"icate","ic"},{"ative",""},{"alize","al"},{"iciti","ic"},
	{"ical","ic"},{"ful",""},{"ness",""}
};

const std::vector<std::string> suff_table = {
	"al","ance","ence","er","ic","able","ible","ant","ement",
	"ment","ent","ism","ate","iti","ous","ive","ize"
};

namespace porter
{
	std::string get_only_letters(const std::string& s) {
		std::string out;
		out.reserve(s.size());
		for (char ch : s) {
			if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
				out.push_back(std::tolower((unsigned char)ch));
			}
		}
		return out;
	}

	bool is_vowel(const std::string& word, int i) {
		char ch = word[i];
		if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') {
			return true;
		}
		if (ch == 'y') {
			if (i == 0) return false;
			return !is_vowel(word, i - 1);
		}
		return false;
	}

	bool contains_vowel(const std::string& word) {
		for (int i = 0; i < (int)word.size(); ++i) {
			if (is_vowel(word, i)) return true;
		}
		return false;
	}

	int vc_count(const std::string& word) {
		int vc_count = 0;
		bool prev_was_vowel = false;
		for (int i = 0; i < (int)word.size(); ++i) {
			bool curr_is_vowel = is_vowel(word, i);
			if (prev_was_vowel && !curr_is_vowel) {
				++vc_count;
			}
			prev_was_vowel = curr_is_vowel;
		}
		return vc_count;
	}

	bool cc_ending(const std::string& word) {
		int word_size = (int)word.size();
		if (word_size < 2) return false;
		if (word[word_size - 1] != word[word_size - 2]) return false;
		return !is_vowel(word, word_size - 1);
	}

	bool cvc_ending(const std::string& word) {
		size_t word_size = word.size();
		if (word_size < 3) return false;
		bool front_c = !is_vowel(word, word_size - 3);
		bool middle_v = is_vowel(word, word_size - 2);
		bool back_c = !is_vowel(word, word_size - 1);
		char last = word[word_size - 1];
		if (front_c && middle_v && back_c && last != 'w' && last != 'x' && last != 'y') return true;
		return false;
	}

	bool suff_ending(const std::string& word, const std::string& suff) {
		if ((int)word.size() < (int)suff.size()) return false;
		return word.compare(word.size() - suff.size(), suff.size(), suff) == 0;
	}

	std::string erase_suffix(const std::string& word, int len) {
		return word.substr(0, word.size() - len);
	}

	void shrink_suff(std::string& word, const suff_table_t& table) {
		for (auto& p : table) {
			if (suff_ending(word, p.first)) {
				std::string stem = erase_suffix(word, (int)p.first.size());
				if (vc_count(stem) > 0)
					word = stem + p.second;
				return;
			}
		}
	}

	void modify_suff(std::string& word) {
		shrink_suff(word, shrink_table_1);
		shrink_suff(word, shrink_table_2);
	}

	std::string get_stem(const std::string& input) {
		std::string word = get_only_letters(input);
		if ((int)word.size() <= 2)
			return word;

		if (suff_ending(word, "sses"))
			word = erase_suffix(word, 2);
		else if (suff_ending(word, "ies"))
			word = erase_suffix(word, 3) + "i";
		else if (suff_ending(word, "s") && word.size() >= 2 && word[word.size() - 2] != 's')
			word = erase_suffix(word, 1);

		int end_length = 0;
		if (suff_ending(word, "ed"))
			end_length = 2;
		else if (suff_ending(word, "ing"))
			end_length = 3;

		std::string base = erase_suffix(word, end_length);
		if (contains_vowel(base))
			word = base;

		if (end_length != 0) {
			if (suff_ending(word, "at") || suff_ending(word, "bl") || suff_ending(word, "iz"))
				word.push_back('e');
			else if (cc_ending(word)) {
				char last = word.back();
				if (last != 'l' && last != 's' && last != 'z')
					word.pop_back();
			}
			else if (vc_count(word) == 1 && cvc_ending(word))
				word.push_back('e');
		}
		if (suff_ending(word, "y")) {
			if (word.size() >= 2 && is_vowel(word, (int)word.size() - 2))
				word.back() = 'i';
		}

		modify_suff(word);

		size_t match_at = -1;
		for (size_t i = 0; i < suff_table.size(); i++) {
			if (suff_ending(word, suff_table[i])) {
				match_at = i;
				break;
			}
		}
		if (match_at != -1) {
			std::string stem = erase_suffix(word, (int)suff_table[match_at].size());
			if (vc_count(stem) > 1)
				word = stem;
		}
		else if (suff_ending(word, "ion")) {
			std::string stem = erase_suffix(word, 3);
			if (!stem.empty()) {
				char ch = stem.back();
				if ((ch == 's' || ch == 't') && vc_count(stem) > 1)
					word = stem;
			}
		}
		if (suff_ending(word, "e")) {
			std::string stem = erase_suffix(word, 1);
			int vc_cnt = vc_count(stem);
			if (vc_cnt > 1 || (vc_cnt == 1 && !cvc_ending(stem)))
				word = stem;
		}
		if (suff_ending(word, "ll") && vc_count(erase_suffix(word, 1)) > 1)
			word.pop_back();

		return word;
	}
};
