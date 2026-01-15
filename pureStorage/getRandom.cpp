#include <iostream>
#include <random>
#include <vector>
using namespace std;

// Custom Hash Table using Separate Chaining (Linked List)
class Node {
  public:
    int key;
    int value; // index in vector
    Node* next;

    Node(int k, int v) : key(k), value(v), next(nullptr) {
    }
};

class CustomHashTable {
  private:
    vector<Node*> buckets;
    int size;
    static const int CAPACITY = 100;

    int hash(int key) {
        return abs(key) % CAPACITY;
    }

  public:
    CustomHashTable() : size(CAPACITY) {
        buckets.resize(size, nullptr);
    }

    void insert(int key, int value) {
        int index = hash(key);
        Node* current = buckets[index];

        // Check if key already exists
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value; // Update value
                return;
            }
            current = current->next;
        }

        // Insert at head
        Node* newNode = new Node(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
    }

    bool find(int key, int& value) {
        int index = hash(key);
        Node* current = buckets[index];

        while (current != nullptr) {
            if (current->key == key) {
                value = current->value;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool erase(int key) {
        int index = hash(key);
        Node* current = buckets[index];
        Node* prev = nullptr;

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    buckets[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    ~CustomHashTable() {
        for (int i = 0; i < size; i++) {
            Node* current = buckets[i];
            while (current != nullptr) {
                Node* temp = current;
                current = current->next;
                delete temp;
            }
        }
    }
};

class RandomizedSet {
  private:
    vector<int> list;
    CustomHashTable indexMap;
    random_device rd;
    mt19937 gen;

  public:
    RandomizedSet() : gen(rd()) {
    }

    bool insert(int val) {
        int idx;
        // O(1) average - hash table lookup
        if (indexMap.find(val, idx)) {
            return false;
        }
        indexMap.insert(val, list.size());
        list.push_back(val);
        return true;
    }

    bool remove(int val) {
        int index;
        // O(1) average - hash table lookup
        if (!indexMap.find(val, index)) {
            return false;
        }

        int lastVal = list.back();

        // Swap with last element
        list[index] = lastVal;
        indexMap.insert(lastVal, index);

        // Remove last element
        list.pop_back();
        indexMap.erase(val);
        return true;
    }

    int getRandom() {
        uniform_int_distribution<> dis(0, list.size() - 1);
        return list[dis(gen)];
    }
};

int main() {
    RandomizedSet rs;

    cout << "Insert 1: " << rs.insert(1) << endl; // true
    cout << "Insert 2: " << rs.insert(2) << endl; // true
    cout << "Insert 3: " << rs.insert(3) << endl; // true
    cout << "Insert 1: " << rs.insert(1) << endl; // false

    cout << "Get random: " << rs.getRandom() << endl; // random from {1, 2, 3}

    cout << "Remove 1: " << rs.remove(1) << endl;     // true
    cout << "Insert 4: " << rs.insert(4) << endl;     // true
    cout << "Get random: " << rs.getRandom() << endl; // random from {2, 3, 4}

    return 0;
}
