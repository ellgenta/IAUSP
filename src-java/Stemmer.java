import java.util.*;

public class Stemmer {
    private static final List<Pair<String,String>> shrinkTable1 = Arrays.asList(
        new Pair<>("ational","ate"),new Pair<>("tional","tion"),new Pair<>("enci","ence"),new Pair<>("anci","ance"),
        new Pair<>("izer","ize"),new Pair<>("abli","able"),new Pair<>("alli","al"),new Pair<>("entli","ent"),
        new Pair<>("eli","e"),new Pair<>("ousli","ous"),new Pair<>("ization","ize"),new Pair<>("ation","ate"),
        new Pair<>("ator","ate"),new Pair<>("alism","al"),new Pair<>("iveness","ive"),new Pair<>("fulness","ful"),
        new Pair<>("ousness","ous")
    );

    private static final List<Pair<String,String>> shrinkTable2 = Arrays.asList(
        new Pair<>("icate","ic"),new Pair<>("ative",""),new Pair<>("alize","al"),new Pair<>("iciti","ic"),
        new Pair<>("ical","ic"),new Pair<>("ful",""),new Pair<>("ness","")
    );

    private static final List<String> suffTable = Arrays.asList(
        "al","ance","ence","er","ic","able","ible","ant","ement",
        "ment","ent","ism","ate","iti","ous","ive","ize"
    );

    // small Pair helper
    static class Pair<A,B> {
        public final A first;
        public final B second;
        Pair(A a, B b){ first = a; second = b; }
    }

    public static String getOnlyLetters(String s) {
        StringBuilder out = new StringBuilder();
        for (char ch : s.toCharArray()) {
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
                out.append(Character.toLowerCase(ch));
            }
        }
        return out.toString();
    }

    private static boolean isVowel(String word, int i) {
        char ch = word.charAt(i);
        if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') return true;
        if (ch == 'y') {
            if (i == 0) return false;
            return !isVowel(word, i - 1);
        }
        return false;
    }

    private static boolean containsVowel(String word) {
        for (int i = 0; i < word.length(); i++) if (isVowel(word, i)) return true;
        return false;
    }

    private static int vcCount(String word) {
        int vc = 0;
        boolean prevWasVowel = false;
        for (int i = 0; i < word.length(); i++) {
            boolean currIsVowel = isVowel(word, i);
            if (prevWasVowel && !currIsVowel) vc++;
            prevWasVowel = currIsVowel;
        }
        return vc;
    }

    private static boolean ccEnding(String word) {
        if (word.length() < 2) return false;
        if (word.charAt(word.length() - 1) != word.charAt(word.length() - 2)) return false;
        return !isVowel(word, word.length() - 1);
    }

    private static boolean cvcEnding(String word) {
        if (word.length() < 3) return false;
        boolean frontC = !isVowel(word, word.length() - 3);
        boolean middleV = isVowel(word, word.length() - 2);
        boolean backC = !isVowel(word, word.length() - 1);
        char last = word.charAt(word.length() - 1);
        if (frontC && middleV && backC && last != 'w' && last != 'x' && last != 'y') return true;
        return false;
    }

    private static boolean suffEnding(String word, String suff) {
        if (word.length() < suff.length()) return false;
        return word.endsWith(suff);
    }

    private static String eraseSuffix(String word, int len) {
        return word.substring(0, word.length() - len);
    }

    private static void shrinkSuff(StringBuilder word, List<Pair<String,String>> table) {
        for (Pair<String,String> p : table) {
            if (suffEnding(word.toString(), p.first)) {
                String stem = eraseSuffix(word.toString(), p.first.length());
                if (vcCount(stem) > 0) {
                    word.setLength(0);
                    word.append(stem).append(p.second);
                }
                return;
            }
        }
    }

    private static void modifySuff(StringBuilder word) {
        shrinkSuff(word, shrinkTable1);
        shrinkSuff(word, shrinkTable2);
    }

    public static String getStem(String input) {
        String word = getOnlyLetters(input);
        if (word.length() <= 2) return word;

        if (suffEnding(word, "sses")) word = eraseSuffix(word, 2);
        else if (suffEnding(word, "ies")) word = eraseSuffix(word, 3) + "i";
        else if (suffEnding(word, "s") && word.length() >= 2 && word.charAt(word.length() - 2) != 's')
            word = eraseSuffix(word, 1);

        int endLength = 0;
        if (suffEnding(word, "ed")) endLength = 2;
        else if (suffEnding(word, "ing")) endLength = 3;

        String base = endLength == 0 ? word : eraseSuffix(word, endLength);
        if (containsVowel(base)) word = base;

        if (endLength != 0) {
            if (suffEnding(word, "at") || suffEnding(word, "bl") || suffEnding(word, "iz"))
                word = word + "e";
            else if (ccEnding(word)) {
                char last = word.charAt(word.length() - 1);
                if (last != 'l' && last != 's' && last != 'z')
                    word = eraseSuffix(word, 1);
            } else if (vcCount(word) == 1 && cvcEnding(word))
                word = word + "e";
        }

        if (suffEnding(word, "y")) {
            if (word.length() >= 2 && isVowel(word, word.length() - 2)) {
                char[] arr = word.toCharArray();
                arr[arr.length - 1] = 'i';
                word = new String(arr);
            }
        }

        StringBuilder sb = new StringBuilder(word);
        modifySuff(sb);
        word = sb.toString();

        int matchAt = -1;
        for (int i = 0; i < suffTable.size(); i++) {
            if (suffEnding(word, suffTable.get(i))) {
                matchAt = i;
                break;
            }
        }
        if (matchAt != -1) {
            String stem = eraseSuffix(word, suffTable.get(matchAt).length());
            if (vcCount(stem) > 1) word = stem;
        } else if (suffEnding(word, "ion")) {
            String stem = eraseSuffix(word, 3);
            if (!stem.isEmpty()) {
                char ch = stem.charAt(stem.length() - 1);
                if ((ch == 's' || ch == 't') && vcCount(stem) > 1) word = stem;
            }
        }

        if (suffEnding(word, "e")) {
            String stem = eraseSuffix(word, 1);
            int vcCnt = vcCount(stem);
            if (vcCnt > 1 || (vcCnt == 1 && !cvcEnding(stem))) word = stem;
        }

        if (suffEnding(word, "ll") && vcCount(eraseSuffix(word, 1)) > 1)
            word = eraseSuffix(word, 1);

        return word;
    }
}
