#define TFIDF_RANKER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <cstddef>

class search_ranking {
public:
    using Term = std::string;
    using TFMap = std::unordered_map<Term, int>;           
    using VectorMap = std::unordered_map<Term, double>;      

    search_ranking(std::unordered_map<Term, double> const& idf, std::vector<TFMap> const& docs_tf)
        : idf_(idf), doc_vectors_()
    {
        precompute_documents(docs_tf);
    }

    search_ranking(std::unordered_map<Term, double> const& idf,
        std::vector<VectorMap> const& docs_tfidf_normalized, bool already_normalized)
        : idf_(idf), doc_vectors_()
    {
        if (already_normalized) {
            doc_vectors_ = docs_tfidf_normalized;
        }
        else {
            // на случай, если передали не-нормализованные веса по ошибке
            doc_vectors_ = docs_tfidf_normalized;
            for (std::size_t i = 0; i < doc_vectors_.size(); ++i) normalize_vector(doc_vectors_[i]);
        }
    }

    std::vector<std::pair<double, std::size_t>> rank_query_from_tf(TFMap const& query_tf,
                                                                   std::size_t topN = 10) const {
        VectorMap qvec;
        double qnorm2 = 0.0;
        for (auto const& p : query_tf) {
            Term const& term = p.first;
            int tf = p.second;
            auto it = idf_.find(term);
            if (it == idf_.end()) continue;
            double w = static_cast<double>(tf) * it->second;
            qvec.emplace(term, w);
            qnorm2 += w * w;
        }
        if (qvec.empty()) return {};

        double qnorm = (qnorm2 > 0.0) ? std::sqrt(qnorm2) : 1.0;
        for (auto& kv : qvec) kv.second /= qnorm;

        std::vector<std::pair<double, std::size_t>> scores;
        scores.reserve(doc_vectors_.size());
        for (std::size_t i = 0; i < doc_vectors_.size(); ++i) {
            double dot = 0.0;
            for (auto const& qp : qvec) {
                auto it = doc_vectors_[i].find(qp.first);
                if (it != doc_vectors_[i].end()) dot += it->second * qp.second;
            }
            scores.emplace_back(dot, i);
        }

        std::sort(scores.begin(), scores.end(),
            [](auto const& a, auto const& b) { return a.first > b.first; });

        if (topN >= scores.size()) return scores;
        std::vector<std::pair<double, std::size_t>> out(scores.begin(), scores.begin() + static_cast<std::ptrdiff_t>(topN));
        return out;
    }

    std::vector<std::pair<double, std::size_t>> rank_query_from_tokens(std::vector<Term> const& tokens,
        std::size_t topN = 10) const
    {
        TFMap qtf;
        for (auto const& t : tokens) qtf[t] += 1;
        return rank_query_from_tf(qtf, topN);
    }

    std::size_t docs_count() const noexcept { return doc_vectors_.size(); }

private:
    std::unordered_map<Term, double> idf_;      
    std::vector<VectorMap> doc_vectors_;          

    void precompute_documents(std::vector<TFMap> const& docs_tf) {
        doc_vectors_.reserve(docs_tf.size());
        for (auto const& tfmap : docs_tf) {
            VectorMap v;
            double norm2 = 0.0;
            for (auto const& p : tfmap) {
                Term const& term = p.first;
                int tf = p.second;
                auto it = idf_.find(term);
                if (it == idf_.end()) continue; 
                double w = static_cast<double>(tf) * it->second;
                v.emplace(term, w);
                norm2 += w * w;
            }
            if (norm2 > 0.0) {
                double norm = std::sqrt(norm2);
                for (auto& kv : v) kv.second /= norm;
            }
            doc_vectors_.push_back(std::move(v));
        }
    }

    static void normalize_vector(VectorMap& v) {
        double norm2 = 0.0;
        for (auto const& kv : v) norm2 += kv.second * kv.second;
        if (norm2 <= 0.0) return;
        double norm = std::sqrt(norm2);
        for (auto& kv : v) kv.second /= norm;
    }
};
