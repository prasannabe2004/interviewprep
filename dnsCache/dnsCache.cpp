#include <cstring>
#include <iostream>

using namespace std;

// Define constants for the number of characters and maximum length
#define CHARS 27
#define MAX 100

// Function to get the index for a character in the trie node's child array
int getIndex(char c) {
    return (c == '.') ? 26 : (c - 'a');
}

// Function to get the character for a given index in the trie node's child array
char getCharFromIndex(int i) {
    return (i == 26) ? '.' : ('a' + i);
}

// Trie Node structure
struct TrieNode {
    bool isLeaf;
    char* ipAdd;
    TrieNode* child[CHARS];
};

// Function to create a new trie node
TrieNode* newTrieNode() {
    TrieNode* newNode = new TrieNode;
    newNode->isLeaf = false;
    newNode->ipAdd = nullptr;
    for (int i = 0; i < CHARS; i++)
        newNode->child[i] = nullptr;
    return newNode;
}

// Function to insert a URL and corresponding IP address into the trie
void insert(TrieNode* root, char* URL, char* ipAdd) {
    int len = strlen(URL);
    TrieNode* pCrawl = root;
    for (int level = 0; level < len; level++) {
        int index = getIndex(URL[level]);

        if (!pCrawl->child[index])
            pCrawl->child[index] = newTrieNode();

        pCrawl = pCrawl->child[index];
    }
    pCrawl->isLeaf = true;
    pCrawl->ipAdd = new char[strlen(ipAdd) + 1];
    strcpy(pCrawl->ipAdd, ipAdd);
}

// Function to search the DNS cache for a given URL and return the corresponding IP address
char* searchDNSCache(TrieNode* root, char* URL) {
    TrieNode* pCrawl = root;
    int len = strlen(URL);

    for (int level = 0; level < len; level++) {
        int index = getIndex(URL[level]);

        if (!pCrawl->child[index])
            return nullptr;

        pCrawl = pCrawl->child[index];
    }

    if (pCrawl != nullptr && pCrawl->isLeaf)
        return pCrawl->ipAdd;

    return nullptr;
}

int main() {
    // Sample data for URLs and corresponding IP addresses
    char URL[][50] = {"www.samsung.com", "www.samsung.net", "www.google.in"};
    char ipAdd[][MAX] = {"107.108.11.123", "107.109.123.255", "74.125.200.106"};
    int n = sizeof(URL) / sizeof(URL[0]);

    // Create the root of the trie
    TrieNode* root = newTrieNode();

    // Insert all the domain names and their corresponding IP addresses into the trie
    for (int i = 0; i < n; i++)
        insert(root, URL[i], ipAdd[i]);

    // Perform forward DNS lookup and print the result
    char url[] = "www.samsung.com";
    char* res_ip = searchDNSCache(root, url);

    if (res_ip != nullptr)
        std::cout << "Forward DNS lookup resolved in cache:\n"
                  << url << " --> " << res_ip << std::endl;
    else
        std::cout << "Forward DNS lookup not resolved in cache" << std::endl;

    return 0;
}