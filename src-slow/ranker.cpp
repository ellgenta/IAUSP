#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include "indexation.cpp"

class search_ranker {
public:
	using term = std::string;
	using tf_map = std::vector<std::pair<std::string, int>>;
	using weight_vector = std::unordered_map<term, double>;
	using score_pair = std::pair<double, size_t>;

	void build(doc_list& docs) {
		docs_tf_.clear();
		inverted_index_.clear();

		for (size_t doc_id = 0; doc_id < docs.size(); ++doc_id) {
			const auto tf = docs[doc_id]->get_tf_map();
			docs_tf_.push_back(tf);
			for (const auto& kv : tf) {
				inverted_index_[kv.first].push_back(doc_id);
			}
		}

		if (docs_tf_.empty()) {
			idf_.clear();
			document_vectors_.clear();
			return;
		}

		calculate_idf();
		build_document_vectors();

		for (auto& kv : inverted_index_) {
			auto& v = kv.second;
			std::sort(v.begin(), v.end());
			v.erase(std::unique(v.begin(), v.end()), v.end());
		}
	}

	std::vector<score_pair> rank_tokens(const std::vector<std::string>& tokens, size_t top_results_count = 10) const {
		if (tokens.empty() || document_vectors_.empty()) return {};

		weight_vector query_vector = build_query_vector(tokens);
		if (query_vector.empty()) return {};

		std::unordered_set<size_t> candidates;
		for (const auto& t : tokens) {
			auto it = inverted_index_.find(t);
			if (it != inverted_index_.end()) {
				candidates.insert(it->second.begin(), it->second.end());
			}
		}

		if (candidates.empty()) return {};

		std::vector<score_pair> document_scores;
		document_scores.reserve(candidates.size());
		for (size_t idx : candidates) {
			double similarity_score = calculate_cosine_similarity(query_vector, document_vectors_[idx]);
			if (similarity_score > 0.0)
				document_scores.emplace_back(similarity_score, idx);
		}

		return get_top_results(document_scores, top_results_count);
	}

private:
	std::vector<tf_map> docs_tf_;
	std::vector<weight_vector> document_vectors_;
	std::unordered_map<term, double> idf_;
	std::unordered_map<term, std::vector<size_t>> inverted_index_;

	void calculate_idf() {
		std::unordered_map<term, int> document_frequency;

		for (const auto& term_frequencies : docs_tf_) {
			for (const auto& kv : term_frequencies) {
				document_frequency[kv.first] += 1;
			}
		}

		idf_.clear();
		idf_.reserve(document_frequency.size());
		const double total_documents = static_cast<double>(docs_tf_.size());

		for (const auto& kv : document_frequency) {
			idf_[kv.first] = std::log(total_documents / (1.0 + static_cast<double>(kv.second))) + 1.0;
		}
	}

	void build_document_vectors() {
		document_vectors_.clear();
		document_vectors_.reserve(docs_tf_.size());

		for (const auto& tf : docs_tf_) {
			weight_vector document_vector = build_document_vector(tf);
			document_vectors_.push_back(std::move(document_vector));
		}
	}

	weight_vector build_document_vector(const tf_map& term_frequencies) const {
		weight_vector document_vector;
		document_vector.reserve(term_frequencies.size());
		double squared_norm = 0.0;

		for (const auto& kv : term_frequencies) {
			const term& term = kv.first;
			int frequency = kv.second;
			auto it = idf_.find(term);
			if (it == idf_.end()) continue;

			const double term_weight = (1.0 + std::log(static_cast<double>(frequency))) * it->second;
			document_vector.emplace(term, term_weight);
			squared_norm += term_weight * term_weight;
		}

		if (squared_norm > 0.0) {
			normalize_vector(document_vector, squared_norm);
		}

		return document_vector;
	}

	weight_vector build_query_vector(const std::vector<std::string>& tokens) const {
		tf_map query_term_frequencies;
		for (const auto& token : tokens) {
			size_t i = 0;
			while(i < query_term_frequencies.size()) {
				if(query_term_frequencies[i].first == token) {
					query_term_frequencies[i].second += 1;
				}
			}
		}

		return build_and_normalize_vector(query_term_frequencies);
	}

	weight_vector build_and_normalize_vector(const tf_map& frequencies) const {
		weight_vector vector;
		double squared_norm = 0.0;

		for (const auto& [term, freq] : frequencies) {
			auto it = idf_.find(term);
			if (it == idf_.end()) continue;

			double weight = (1.0 + std::log(static_cast<double>(freq))) * it->second;
			vector[term] = weight;
			squared_norm += weight * weight;
		}

		if (squared_norm > 0.0) {
			normalize_vector(vector, squared_norm);
		}

		return vector;
	}

	void normalize_vector(weight_vector& vector) const {
		double squared_norm = 0.0;
		for (const auto& kv : vector) squared_norm += kv.second * kv.second;
		normalize_vector(vector, squared_norm);
	}

	void normalize_vector(weight_vector& vector, double squared_norm) const {
		if (squared_norm <= 0.0) return;
		const double norm = std::sqrt(squared_norm);
		for (auto& kv : vector) kv.second /= norm;
	}

	double calculate_cosine_similarity(const weight_vector& query_vector, const weight_vector& document_vector) const {
		double dot_product = 0.0;
		for (const auto& kv : query_vector) {
			auto it = document_vector.find(kv.first);
			if (it != document_vector.end()) {
				dot_product += it->second * kv.second;
			}
		}
		return dot_product;
	}

	std::vector<score_pair> get_top_results(std::vector<score_pair>& scores, size_t top_results_count) const {
		if (scores.empty()) return {};

		if (top_results_count >= scores.size()) {
			std::sort(scores.begin(), scores.end(),
				[](const score_pair& a, const score_pair& b) { return a.first > b.first; });
			return scores;
		}

		std::partial_sort(scores.begin(), scores.begin() + top_results_count, scores.end(),
			[](const score_pair& a, const score_pair& b) { return a.first > b.first; });

		return std::vector<score_pair>(scores.begin(), scores.begin() + top_results_count);
	}
};
