#include <iostream>

using namespace std;

/*
Implement a Trie data structure with insert and search functionalities.
*/

struct TrieNode {
    TrieNode* children[26];
    bool isEndOfWord;
};

class Trie {
  private:
    TrieNode* root;

  public:
    Trie() {
        root = new TrieNode();
        for (int i = 0; i < 26; i++) {
            root->children[i] = nullptr;
        }
        root->isEndOfWord = false;
    }

    void insert(string word) {
        TrieNode* node = root;
        for (char c : word) {
            if (!node->children[c - 'a']) {
                node->children[c - 'a'] = new TrieNode();
            }
            node = node->children[c - 'a'];
        }
        node->isEndOfWord = true;
    }
    bool search(string word) {
        TrieNode* node = root;
        for (char c : word) {
            if (!node->children[c - 'a']) {
                return false;
            }
            node = node->children[c - 'a'];
        }
        return node->isEndOfWord;
    }
};

int main() {
    Trie trie;
    trie.insert("apple");
    trie.insert("app");
    trie.insert("banana");
    cout << trie.search("apple") << endl;  // Output: 1
    cout << trie.search("app") << endl;    // Output: 1
    cout << trie.search("banana") << endl; // Output: 1
    cout << trie.search("orange") << endl; // Output: 0
    cout << trie.search("a") << endl;      // Output: 0
    return 0;
}