import java.util.*;

public class Tokenizer {
    public static List<String> tokenize(String text) {
        List<String> tokens = new ArrayList<>();
        StringBuilder temp = new StringBuilder();
        for (char ch : text.toCharArray()) {
            if (Character.isLetterOrDigit(ch)) {
                temp.append(Character.toLowerCase(ch));
            } else {
                if (temp.length() > 0) {
                    tokens.add(temp.toString());
                    temp.setLength(0);
                }
            }
        }
        if (temp.length() > 0) tokens.add(temp.toString());
        return tokens;
    }

    public static List<String> stemTokens(List<String> tokens) {
        List<String> out = new ArrayList<>();
        for (String t : tokens) {
            String s = Stemmer.getStem(t);
            if (!s.isEmpty()) out.add(s);
        }
        return out;
    }

    public static List<String> getTokens(String text) {
        List<String> toks = tokenize(text);
        return stemTokens(toks);
    }
}
