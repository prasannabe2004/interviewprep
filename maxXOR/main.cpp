#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class TrieNode {
public:
    TrieNode* child[2];

    TrieNode() {
        child[0] = nullptr;
        child[1] = nullptr;
    }
};

class Trie {
    TrieNode* root;

public:
    Trie() {
        root = new TrieNode();
    }

    void insert(int num) {
        TrieNode* node = root;
        for (int i = 31; i >= 0; i--) { // Assuming 32-bit integers
            int bit = (num >> i) & 1;
            if (!node->child[bit]) {
                node->child[bit] = new TrieNode();
            }
            node = node->child[bit];
        }
    }

    int getMaxXOR(int num) {
        TrieNode* node = root;
        int maxXor = 0;
        for (int i = 31; i >= 0; i--) {
            int bit = (num >> i) & 1;
            int oppositeBit = 1 - bit;
            if (node->child[oppositeBit]) {
                maxXor |= (1 << i);
                node = node->child[oppositeBit];
            } else {
                node = node->child[bit];
            }
        }
        return maxXor;
    }
};

int findMaximumXOR(vector<int>& nums) {
    Trie trie;
    int maxResult = 0;

    for (int num : nums) {
        trie.insert(num);
    }

    for (int num : nums) {
        maxResult = max(maxResult, trie.getMaxXOR(num));
    }

    return maxResult;
}

int main() {
    vector<int> nums = {3, 10, 5, 25, 2, 8};
    cout << "Maximum XOR: " << findMaximumXOR(nums) << endl;
    return 0;
}
