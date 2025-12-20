import java.util.*;

class TrieNode {
    TrieNode[] ch;
    boolean isTerm = false;
    int count = 0;

    static final int CH_SIZE = 26;

    TrieNode() {
        ch = new TrieNode[CH_SIZE];
        for (int i = 0; i < CH_SIZE; i++) ch[i] = null;
    }

    boolean isLeaf() {
        for (int i = 0; i < CH_SIZE; i++) {
            if (ch[i] != null) return false;
        }
        return true;
    }
}

public class Trie {
    private TrieNode root;
    private int chCount = 0;

    public Trie() { root = new TrieNode(); }

    public Trie(List<String> list) {
        root = new TrieNode();
        for (String s : list) insert(s);
    }

    // preserve the original C++ index mapping: (sym % 'a')
    private int getIndex(char sym) {
        return (int) sym % (int) 'a';
    }

    private TrieNode _find(TrieNode node, String stem, int i) {
        if (node == null) return null;
        if (i >= stem.length()) return node;
        int idx = getIndex(stem.charAt(i));
        if (node.ch[idx] != null) return _find(node.ch[idx], stem, i + 1);
        return null;
    }

    private void _pushPrefix(TrieNode stNode, String stem, int st) {
        if (st == stem.length()) {
            stNode.count += 1;
            stNode.isTerm = true;
            return;
        }
        int idx = getIndex(stem.charAt(st));
        stNode.ch[idx] = new TrieNode();
        _pushPrefix(stNode.ch[idx], stem, st + 1);
    }

    private void _insert(TrieNode node, String stem, int i) {
        if (node == null) return;
        if (i == stem.length()) {
            node.count += 1;
            node.isTerm = true;
            return;
        }
        int idx = getIndex(stem.charAt(i));
        if (node.ch[idx] != null) {
            _insert(node.ch[idx], stem, i + 1);
        } else {
            if (node == this.root) chCount += 1;
            _pushPrefix(node, stem, i);
        }
    }

    private void _erase(TrieNode node, String stem, int i) {
        if (node == null) return;
        if (i == stem.length()) {
            node.count -= 1;
            if (node.count == 0) node.isTerm = false;
            return;
        }
        int idx = getIndex(stem.charAt(i));
        if (node.ch[idx] != null) {
            _erase(node.ch[idx], stem, i + 1);
            if (node.ch[idx].isLeaf()) {
                node.ch[idx] = null;
            }
        }
    }

    private void _clear(TrieNode node) {
        if (node == null) return;
        for (int i = 0; i < TrieNode.CH_SIZE; i++) {
            if (node.ch[i] != null) _clear(node.ch[i]);
        }
        // allow GC to reclaim by dropping references
    }

    private void traverse(Map<String, Integer> tfMap, TrieNode node, StringBuilder temp) {
        for (int i = 0; i < TrieNode.CH_SIZE; i++) {
            if (node.ch[i] != null) {
                temp.append((char)('a' + i));
                traverse(tfMap, node.ch[i], temp);
                if (node.ch[i].isTerm) {
                    tfMap.put(temp.toString(), node.ch[i].count);
                }
                temp.setLength(temp.length() - 1);
            }
        }
    }

    private void _getBytesCount(TrieNode node, long[] bytes) {
        if (node == null) return;
        for (int i = 0; i < TrieNode.CH_SIZE; i++) {
            if (node.ch[i] != null) _getBytesCount(node.ch[i], bytes);
        }
        // estimate size of node object
        bytes[0] += 16 + TrieNode.CH_SIZE * 8; // rough estimate
    }

    public TrieNode find(String stem) { return _find(root, stem, 0); }

    public void insert(String stem) { if (stem == null) return; _insert(root, stem, 0); }

    public void erase(String stem) { if (stem == null) return; _erase(root, stem, 0); }

    public boolean empty() { return chCount == 0; }

    public boolean contains(String stem) {
        TrieNode node = find(stem);
        if (node == null) return false;
        return node.count != 0;
    }

    public int count(String stem) {
        TrieNode node = find(stem);
        if (node == null) return 0;
        return node.count;
    }

    public int count(List<String> stems) {
        int ans = 0;
        for (String s : stems) {
            TrieNode t = find(s);
            if (t != null) ans += t.count;
        }
        return ans;
    }

    public Map<String, Integer> getTfMap() {
        Map<String, Integer> tfMap = new HashMap<>();
        StringBuilder temp = new StringBuilder();
        traverse(tfMap, root, temp);
        return tfMap;
    }

    public void clear() {
        _clear(root);
        root = null;
    }

    public long getBytesCount() {
        long[] bytes = new long[]{0};
        if (this.root != null) _getBytesCount(this.root, bytes);
        bytes[0] += 24; // estimate for the Trie object itself
        return bytes[0];
    }
}
