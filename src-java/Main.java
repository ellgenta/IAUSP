import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.stream.Collectors;

public class Main {
    static String readFile(Path p) {
        try {
            byte[] bytes = Files.readAllBytes(p);
            return new String(bytes);
        } catch (IOException e) {
            System.err.println("Warning: cannot open file: " + p.toString());
            return "";
        }
    }

    public static class SnippetGenerator {
        public static final int CONTEXT_BEFORE = 40;
        public static final int CONTEXT_AFTER = 120;
        public static final int MAX_SNIPPET_LEN = CONTEXT_BEFORE + CONTEXT_AFTER;

        private static String toLowerCase(String s) {
            return s.toLowerCase();
        }

        private static int findFirstMatch(String textLower, Set<String> terms) {
            int best = -1;
            for (String term : terms) {
                if (term.isEmpty()) continue;
                int pos = textLower.indexOf(term);
                if (pos != -1) {
                    if (best == -1 || pos < best) best = pos;
                }
            }
            return best;
        }

        private static String highlightTerms(String snippet, String snippetLower, Set<String> terms) {
            StringBuilder result = new StringBuilder(snippet);
            int offset = 0;
            List<String> sortedTerms = new ArrayList<>(terms);
            sortedTerms.sort((a,b)-> Integer.compare(b.length(), a.length()));
            for (String term : sortedTerms) {
                if (term.isEmpty()) continue;
                int pos = 0;
                while (pos < snippetLower.length()) {
                    int found = snippetLower.indexOf(term, pos);
                    if (found == -1) break;
                    int adjustedPos = found + offset;
                    result.insert(adjustedPos, "[");
                    int endIdx = adjustedPos + 1;
                    while (endIdx < result.length() && Character.isAlphabetic(result.charAt(endIdx))) endIdx++;
                    result.insert(endIdx, "]");
                    offset += 2;
                    pos = found + term.length();
                }
            }
            return result.toString();
        }

        private static String extractContextWithHighlight(String originalText, String textLower, int matchPosition, Set<String> terms) {
            int start = Math.max(0, matchPosition - CONTEXT_BEFORE);
            int end = Math.min(matchPosition + CONTEXT_AFTER, originalText.length());
            String snippet = originalText.substring(start, end);
            String snippetLower = textLower.substring(start, end);
            String highlighted = highlightTerms(snippet, snippetLower, terms);
            if (start > 0) highlighted = "..." + highlighted;
            if (end < originalText.length()) highlighted = highlighted + "...";
            return highlighted;
        }

        private static String truncateText(String text, int maxLength) {
            if (text.length() <= maxLength) return text;
            return text.substring(0, maxLength) + "...";
        }

        public static String makeSnippet(Doc doc, List<String> queryTokens) {
            Path path = Paths.get(doc.getPath());
            String text = readFile(path);
            if (text.isEmpty() || queryTokens.isEmpty()) return truncateText(text, MAX_SNIPPET_LEN);

            Set<String> searchTerms = new HashSet<>();
            for (String token : queryTokens) {
                String stemmed = Stemmer.getStem(token);
                if (!stemmed.isEmpty()) searchTerms.add(stemmed);
                searchTerms.add(token);
            }

            String textLower = toLowerCase(text);
            int bestPos = findFirstMatch(textLower, searchTerms);
            if (bestPos == -1) return truncateText(text, MAX_SNIPPET_LEN);
            return extractContextWithHighlight(text, textLower, bestPos, searchTerms);
        }
    }

    public static void main(String[] args) {
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(System.in))) {
            System.out.println("Enter folder path to scan for .txt files (empty = current dir):");
            System.out.print("> ");
            String folderPath = reader.readLine();
            if (folderPath == null || folderPath.isEmpty()) folderPath = ".";

            System.out.print("How many top results to show for each query? [default 10]: ");
            String results = reader.readLine();
            int shownResultsCount = 10;
            if (results != null && !results.isEmpty()) {
                try { shownResultsCount = Integer.parseUnsignedInt(results); }
                catch (Exception ex) { shownResultsCount = 10; }
            }

            Path root = Paths.get(folderPath);
            if (!Files.exists(root) || !Files.isDirectory(root)) {
                System.err.println("Folder not found or not a directory: " + folderPath);
                return;
            }

            List<Path> found = new ArrayList<>();
            try (DirectoryStream<Path> ds = Files.newDirectoryStream(root)) {
                for (Path p : ds) {
                    if (!Files.isRegularFile(p)) continue;
                    String ext = p.getFileName().toString();
                    int dot = ext.lastIndexOf('.');
                    if (dot != -1) ext = ext.substring(dot).toLowerCase();
                    else ext = "";
                    if (ext.equals(".txt")) found.add(p);
                }
            } catch (IOException e) {
                System.err.println("Warning: cannot start directory iteration: " + e.getMessage());
            }

            if (found.isEmpty()) {
                System.err.println("No .txt files found under " + folderPath);
                return;
            }

            System.out.println("Found " + found.size() + " .txt files. Indexing...");
            DocList docs = new DocList();

            long tBefore = System.nanoTime();
            for (Path fp : found) {
                try {
                    String text = readFile(fp);
                    if (text.isEmpty()) {
                        try {
                            long sz = Files.size(fp);
                            if (sz == 0) {
                                System.err.println("Info: skipping empty file " + fp.toString());
                                continue;
                            }
                        } catch (IOException ex) {
                            System.err.println("Warning: cannot stat file " + fp.toString() + " : " + ex.getMessage());
                            continue;
                        }
                    }
                    Doc d = new Doc(fp.toString(), text);
                    docs.add(d);
                } catch (Exception ex) {
                    System.err.println("Warning: exception reading file " + fp.toString() + " : " + ex.getMessage() + " -- skipping");
                }
            }
            long tAfter = System.nanoTime();
            System.out.printf("Tokenization + Indexation: %.5f ms%n", (tAfter - tBefore) / 1_000_000.0);

            if (docs.isEmpty()) {
                System.err.println("No readable documents to index.");
                return;
            }

            SearchRanker ranker = new SearchRanker();
            try {
                ranker.build(docs);
            } catch (Exception ex) {
                System.err.println("Error building index: " + ex.getMessage());
            }

            System.out.println("Indexing done. Enter queries (empty line to skip).\n");
            while (true) {
                System.out.print("Query> ");
                String userInput = reader.readLine();
                if (userInput == null) break;
                if (userInput.isEmpty()) continue;

                long qBefore = System.nanoTime();
                List<String> qtokens = Tokenizer.getTokens(userInput);
                if (qtokens.isEmpty()) {
                    System.out.println("(no valid tokens)");
                    continue;
                }
                List<ScorePair> scores = ranker.rankTokens(qtokens, shownResultsCount);
                if (scores.isEmpty()) {
                    System.out.println("No matching documents.");
                    continue;
                }
                long qAfter = System.nanoTime();
                System.out.printf("Query analysis: %.5f ms%n", (qAfter - qBefore) / 1_000_000.0);

                int r = 0;
                for (ScorePair sp : scores) {
                    r++;
                    int docidx = sp.docIndex;
                    if (docidx >= docs.size()) continue;
                    System.out.println(r + ". [" + sp.score + "] " + docs.get(docidx).getPath());
                    String snippet = SnippetGenerator.makeSnippet(docs.get(docidx), qtokens);
                    if (snippet.length() > 150) {
                        System.out.println("<" + snippet.substring(0, 150) + ">");
                    } else {
                        System.out.println("<" + snippet);
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
