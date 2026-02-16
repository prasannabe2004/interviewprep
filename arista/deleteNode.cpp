#include <iostream>

using namespace std;

/*
Write a function to delete a node in a singly linked list, given only access to that node.
*/

struct ListNode {
    int val;
    struct ListNode* next;
    ListNode(int v) : val(v), next(nullptr) {
    }
};

class SingleLinkedList {
  private:
    ListNode* head;

  public:
    SingleLinkedList() {
        head = nullptr;
    }

    // Destructor to free the allocated memory for the list nodes
    ~SingleLinkedList() {
        ListNode* current = head;
        while (current != nullptr) {
            ListNode* next = current->next;
            delete current;
            current = next;
        }
    }

    void insert(int val) {
        ListNode* n = new ListNode(val);
        if (head == nullptr) {
            head = n;
            return;
        }
        ListNode* curr = head;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = n;
    }

    void printList(void) {
        ListNode* curr = head;
        while (curr) {
            cout << curr->val << " ";
            curr = curr->next;
        }
        cout << endl;
    }

    // Helper to get a node for testing deletion
    ListNode* getHead() {
        return head;
    }

    void deleteNode(ListNode* node) {
        if (node == nullptr) {
            return;
        }
        ListNode* next = node->next;
        node->val = next->val;
        node->next = next->next;
        delete next;
    }
};

int main() {
    SingleLinkedList list;
    list.insert(1);
    list.insert(2);
    list.insert(3);
    list.insert(4);
    cout << "Original list: ";
    list.printList(); // Expected: 1 2 3 4

    // Get a pointer to the node with value 2 to test deletion
    ListNode* node_to_delete = list.getHead()->next;
    cout << "Deleting node with value " << node_to_delete->val << "..." << endl;
    list.deleteNode(node_to_delete);

    cout << "List after deletion: ";
    list.printList(); // Expected: 1 3 4
    return 0;
}