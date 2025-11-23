import java.util.*;
import java.util.stream.Collectors;

class ScorePair {
    public final double score;
    public final int docIndex;
    public ScorePair(double s, int idx){ score = s; docIndex = idx; }
}

public class SearchRanker {
    private final List<Map<String,Integer>> docsTf = new ArrayList<>();
    private final List<Map<String,Double>> documentVectors = new ArrayList<>();
    private final Map<String, Double> idf = new HashMap<>();
    private final Map<String, List<Integer>> invertedIndex = new HashMap<>();

    private void calculateIdf() {
        Map<String,Integer> documentFrequency = new HashMap<>();
        for (Map<String,Integer> tf : docsTf) {
            for (String term : tf.keySet()) {
                documentFrequency.put(term, documentFrequency.getOrDefault(term, 0) + 1);
            }
        }
        idf.clear();
        double totalDocuments = (double) docsTf.size();
        for (Map.Entry<String,Integer> kv : documentFrequency.entrySet()) {
            idf.put(kv.getKey(), Math.log(totalDocuments / (1.0 + kv.getValue())) + 1.0);
        }
    }

    private void buildDocumentVectors() {
        documentVectors.clear();
        for (Map<String,Integer> tf : docsTf) {
            Map<String,Double> docVector = buildDocumentVector(tf);
            documentVectors.add(docVector);
        }
    }

    private Map<String,Double> buildDocumentVector(Map<String,Integer> termFrequencies) {
        Map<String,Double> documentVector = new HashMap<>();
        double squaredNorm = 0.0;
        for (Map.Entry<String,Integer> kv : termFrequencies.entrySet()) {
            String term = kv.getKey();
            int frequency = kv.getValue();
            Double idfVal = idf.get(term);
            if (idfVal == null) continue;
            double termWeight = (1.0 + Math.log((double) frequency)) * idfVal;
            documentVector.put(term, termWeight);
            squaredNorm += termWeight * termWeight;
        }
        if (squaredNorm > 0.0) normalizeVector(documentVector, squaredNorm);
        return documentVector;
    }

    private Map<String,Double> buildQueryVector(List<String> tokens) {
        Map<String,Integer> qtf = new HashMap<>();
        for (String t : tokens) qtf.put(t, qtf.getOrDefault(t, 0) + 1);
        return buildAndNormalizeVector(qtf);
    }

    private Map<String,Double> buildAndNormalizeVector(Map<String,Integer> freqs) {
        Map<String,Double> vector = new HashMap<>();
        double squaredNorm = 0.0;
        for (Map.Entry<String,Integer> e : freqs.entrySet()) {
            Double idfVal = idf.get(e.getKey());
            if (idfVal == null) continue;
            double w = (1.0 + Math.log((double) e.getValue())) * idfVal;
            vector.put(e.getKey(), w);
            squaredNorm += w * w;
        }
        if (squaredNorm > 0.0) normalizeVector(vector, squaredNorm);
        return vector;
    }

    private void normalizeVector(Map<String,Double> vector) {
        double sq = 0.0;
        for (double v : vector.values()) sq += v * v;
        normalizeVector(vector, sq);
    }

    private void normalizeVector(Map<String,Double> vector, double squaredNorm) {
        if (squaredNorm <= 0.0) return;
        double norm = Math.sqrt(squaredNorm);
        for (Map.Entry<String,Double> e : new ArrayList<>(vector.entrySet())) {
            vector.put(e.getKey(), e.getValue() / norm);
        }
    }

    private double calculateCosineSimilarity(Map<String,Double> queryVector, Map<String,Double> documentVector) {
        double dot = 0.0;
        for (Map.Entry<String,Double> kv : queryVector.entrySet()) {
            Double v = documentVector.get(kv.getKey());
            if (v != null) dot += v * kv.getValue();
        }
        return dot;
    }

    private List<ScorePair> getTopResults(List<ScorePair> scores, int topResultsCount) {
        if (scores.isEmpty()) return Collections.emptyList();
        if (topResultsCount >= scores.size()) {
            scores.sort((a,b)-> Double.compare(b.score, a.score));
            return scores;
        }
        scores.sort((a,b)-> Double.compare(b.score, a.score));
        return new ArrayList<>(scores.subList(0, topResultsCount));
    }

    public void build(DocList docs) {
        docsTf.clear();
        invertedIndex.clear();

        for (int docId = 0; docId < docs.size(); docId++) {
            Map<String,Integer> tf = docs.get(docId).getContent().getTfMap();
            docsTf.add(tf);
            for (String term : tf.keySet()) {
                invertedIndex.computeIfAbsent(term, k -> new ArrayList<>()).add(docId);
            }
        }

        if (docsTf.isEmpty()) {
            idf.clear();
            documentVectors.clear();
            return;
        }

        calculateIdf();
        buildDocumentVectors();

        for (List<Integer> v : invertedIndex.values()) {
            Collections.sort(v);
            v.replaceAll(Integer::valueOf);
            // remove duplicates
            List<Integer> uniq = v.stream().distinct().collect(Collectors.toList());
            v.clear();
            v.addAll(uniq);
        }
    }

    public List<ScorePair> rankTokens(List<String> tokens, int topResultsCount) {
        if (tokens.isEmpty() || documentVectors.isEmpty()) return Collections.emptyList();

        Map<String,Double> queryVector = buildQueryVector(tokens);
        if (queryVector.isEmpty()) return Collections.emptyList();

        Set<Integer> candidates = new HashSet<>();
        for (String t : tokens) {
            List<Integer> l = invertedIndex.get(t);
            if (l != null) candidates.addAll(l);
        }

        if (candidates.isEmpty()) return Collections.emptyList();

        List<ScorePair> documentScores = new ArrayList<>();
        for (int idx : candidates) {
            double sim = calculateCosineSimilarity(queryVector, documentVectors.get(idx));
            if (sim > 0.0) documentScores.add(new ScorePair(sim, idx));
        }

        return getTopResults(documentScores, topResultsCount);
    }
}
