#include <iostream>
#include <queue>
using namespace std;

/*
Find the position (1-based rank in inorder traversal) of a given number in a Binary Search Tree
(BST).
*/

// Node structure for BST
struct Node {
    int data;
    Node* left;
    Node* right;
    Node(int val) : data(val), left(nullptr), right(nullptr) {
    }
};

class Solution {
  public:
    // Function to find the position (level) of a given key in BST
    int findPosition(Node* root, int key) {
        int level = 1; // Root is at level 1
        Node* current = root;

        while (current) {
            if (key == current->data)
                return level; // Found the key
            else if (key < current->data)
                current = current->left;
            else
                current = current->right;
            level++;
        }
        return -1; // Key not found
    }
};

int main() {
    Solution s;
    /*
                    5
                3       7
            2       4 6    8
        1
    0
    */
    Node* root = new Node(5);
    root->left = new Node(3);
    root->right = new Node(7);
    root->left->left = new Node(2);
    root->left->right = new Node(4);
    root->right->left = new Node(6);
    root->right->right = new Node(8);
    root->left->left->left = new Node(1);
    root->left->left->left->left = new Node(0);
    cout << s.findPosition(root, 0) << endl; // Output: 3
    return 0;
}
