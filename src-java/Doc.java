import java.util.*;

public class Doc {
    private final String path;
    private final Trie content;

    public Doc(String path, String text) {
        this.path = path;
        this.content = new Trie();
        List<String> tokens = Tokenizer.getTokens(text);
        for (String t : tokens) content.insert(t);
    }

    // copy-like constructors (shallow as в C++)
    public Doc(Doc other) {
        this.path = other.path;
        this.content = other.content;
    }

    public String getPath() { return path; }

    public Trie getContent() { return content; }

    public long getBytesCount() {
        long bytes = 0;
        bytes += path.length() * 2 + 24; // rough estimate of String
        bytes += content.getBytesCount();
        return bytes;
    }
}

class DocList {
    private final List<Doc> list = new ArrayList<>();

    public DocList() {}

    public void add(Doc d) { list.add(d); }

    public Doc get(int idx) { return list.get(idx); }

    public boolean isEmpty() { return list.isEmpty(); }

    public int size() { return list.size(); }

    public Doc back() { return list.get(list.size() - 1); }

    public List<Doc> asList() { return list; }
}
