#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <utility>
#include <cctype>

class porter_stemmer_tokenization
{
private:
	std::string preprocess(const std::string& s) {
		std::string out;
		out.reserve(s.size());

		for (char ch : s) {
			if (is_letter(ch)) out.push_back(::tolower(ch));
		}
		return out;
	}

	static bool is_letter(char ch) {
		return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
	}

	bool is_vowel(const std::string& w, int i) {
		char ch = w[i];
		if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') return true;
		if (ch == 'y') {
			if (i == 0) return false; // y в начале слова - согласная
			// y — гласная, если слева согласная
			return !is_vowel(w, i - 1);
		}
		return false;
	}

	bool contains_vowel(const std::string& w) {
		for (int i = 0; i < (int)w.size(); ++i) if (is_vowel(w, i)) return true;
		return false;
	}

	int measure(const std::string& w) {
		if (w.empty()) return 0;
		std::string pat;
		pat.reserve(w.size());
		for (int i = 0; i < (int)w.size(); ++i) pat.push_back(is_vowel(w, i) ? 'V' : 'C');
		int m = 0;
		for (int i = 0; i + 1 < (int)pat.size(); ++i) if (pat[i] == 'V' && pat[i + 1] == 'C') ++m;
		return m;
	}

	bool ends_with_double_consonant(const std::string& w) {
		int n = w.size();
		if (n < 2) return false;
		char a = w[n - 1], b = w[n - 2];
		if (a != b) return false;
		return !is_vowel(w, n - 1);
	}

	bool cvc_ending(const std::string& w) {
		int n = w.size();
		if (n < 3) return false;
		bool c1 = !is_vowel(w, n - 3);
		bool v = is_vowel(w, n - 2);
		bool c2 = !is_vowel(w, n - 1);
		char last = w[n - 1];
		if (c1 && v && c2 && last != 'w' && last != 'x' && last != 'y') return true;
		return false;
	}

	bool ends_with(const std::string& w, const std::string& suf) {
		if ((int)w.size() < (int)suf.size()) return false;
		return w.compare(w.size() - suf.size(), suf.size(), suf) == 0;
	}

	std::string remove_suffix(const std::string& word, int length) {
		return word.substr(0, word.size() - length);
	}
public:
	std::string tokenize(const std::string& word_in) {
		std::string word = preprocess(word_in);
		if ((int)word.size() <= 2) return word;

		// --- Step 1a: множественное число ---
		if (ends_with(word, "sses")) {
			word = remove_suffix(word, 2);
		}
		else if (ends_with(word, "ies")) {
			word = remove_suffix(word, 3) + "i";
		}
		else if (ends_with(word, "s")) {
			if (word[word.size() - 2] != 's') word = remove_suffix(word, 1);
		}

		// --- Step 1b: ed / ing ---
		bool didStep1b = false;
		if (ends_with(word, "ed")) {
			std::string base = remove_suffix(word, 2);
			if (contains_vowel(base)) {
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

		if (didStep1b) {
			if (ends_with(word, "at") || ends_with(word, "bl") || ends_with(word, "iz")) {
				word += 'e';
			}
			else if (ends_with_double_consonant(word)) {
				char last = word.back();
				if (last != 'l' && last != 's' && last != 'z') word.pop_back();
			}
			else if (measure(word) == 1 && cvc_ending(word)) {
				word += 'e';
			}
		}

		// --- Step 1c: trailing y -> i (если перед ним гласная) ---
		if (ends_with(word, "y")) {
			if (word.size() >= 2 && is_vowel(word, word.size() - 2)) {
				word.back() = 'i';
			}
		}

		// --- Step 2: длинные суффиксы (только если measure > 0) ---
		const std::vector<std::pair<std::string, std::string>> step2 = {
			{"ational","ate"},{"tional","tion"},{"enci","ence"},{"anci","ance"},
			{"izer","ize"},{"abli","able"},{"alli","al"},{"entli","ent"},
			{"eli","e"},{"ousli","ous"},{"ization","ize"},{"ation","ate"},
			{"ator","ate"},{"alism","al"},{"iveness","ive"},{"fulness","ful"},
			{"ousness","ous"} // можно расширить
		};
		for (auto& p : step2) {
			const std::string& suf = p.first;
			const std::string& rep = p.second;
			if (ends_with(word, suf)) {
				std::string stem = remove_suffix(word, suf.size());
				if (measure(stem) > 0) {
					word = stem + rep;
				}
				break; // применяем только первое подходящее правило
			}
		}

		// --- Step 3: ещё замены (m > 0) ---
		const std::vector<std::pair<std::string, std::string>> step3 = {
			{"icate","ic"},{"ative",""}, {"alize","al"}, {"iciti","ic"},
			{"ical","ic"}, {"ful",""}, {"ness",""}
		};
		for (auto& p : step3) {
			const std::string& suf = p.first;
			const std::string& rep = p.second;
			if (ends_with(word, suf)) {
				std::string stem = remove_suffix(word, suf.size());
				if (measure(stem) > 0) {
					word = stem + rep;
				}
				break;
			}
		}

		// --- Step 4: удаляем короткие суффиксы при measure > 1 ---
		const std::vector<std::string> step4 = {
			"al","ance","ence","er","ic","able","ible","ant","ement",
			"ment","ent","ism","ate","iti","ous","ive","ize"
		};
		bool removed_step4 = false;
		for (auto& suf : step4) {
			if (ends_with(word, suf)) {
				std::string stem = remove_suffix(word, suf.size());
				if (measure(stem) > 1) {
					word = stem;
				}
				removed_step4 = true;
				break;
			}
		}
		// Особое правило для "ion": убрать, если перед ion стоит 's' или 't' и measure>1
		if (!removed_step4 && ends_with(word, "ion")) {
			std::string stem = remove_suffix(word, 3);
			if (!stem.empty()) {
				char ch = stem.back();
				if ((ch == 's' || ch == 't') && measure(stem) > 1) {
					word = stem;
				}
			}
		}

		// --- Step 5: финальная корректировка ---
		// удаляем конечное 'e' при measure>1 или (measure==1 && not cvc)
		if (ends_with(word, "e")) {
			std::string stem = remove_suffix(word, 1);
			int m = measure(stem);
			if (m > 1 || (m == 1 && !cvc_ending(stem))) {
				word = stem;
			}
		}
		// уменьшение "ll" до "l" если measure > 1
		if (ends_with(word, "ll") && measure(remove_suffix(word, 1)) > 1) {
			word.pop_back();
		}

		return word;
	}
};
