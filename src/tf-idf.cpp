#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include "indexation.cpp"

class SearchRanker {
public:
	using Term = std::string;
	using TermFrequencyMap = std::unordered_map<Term, int>;
	using WeightVector = std::unordered_map<Term, double>;
	using ScorePair = std::pair<double, size_t>;

	void build(std::vector<doc_t>& docs) {
		for(auto& d : docs) 
			documents_term_frequencies_.push_back(d.get_tf_map());

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

	std::vector<ScorePair> calculate_document_scores(const WeightVector& query_vector) const {
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

	double calculate_cosine_similarity(const WeightVector& query_vector, const WeightVector& document_vector) const {
		double dot_product = 0.0;
		for (const auto& kv : query_vector) {
			auto it = document_vector.find(kv.first);
			if (it != document_vector.end()) {
				dot_product += it->second * kv.second;
			}
		}
		return dot_product;
	}

	std::vector<ScorePair> get_top_results(std::vector<ScorePair>& scores, size_t top_results_count) const {
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
