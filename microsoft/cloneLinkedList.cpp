#include <iostream>
#include <unordered_map>
using namespace std;

class Node {
  public:
    int data;
    Node* next;
    Node* random;

    Node(int x) {
        data = x;
        next = random = NULL;
    }
};

class Solution {
  public:
    /*
      Time Complexity: O(N), where N is the number of nodes in the linked list.
      Space Complexity: O(1).
    */
    Node* cloneLinkedList2(Node* head) {
        if (head == NULL)
            return NULL;

        // Creating a new weaved list of original and copied nodes.
        Node* curr = head;
        while (curr != NULL) {
            Node* temp = new Node(curr->data);
            temp->next = curr->next;
            curr->next = temp;
            curr = temp->next;
        }

        curr = head;

        // Now link the random pointers of the new nodes created.
        while (curr != NULL) {
            if (curr->random != NULL)
                curr->next->random = curr->random->next;
            curr = curr->next->next;
        }

        // Unweave the linked list to get back the original linked list
        // and the cloned linked list.
        Node *original = head, *copy = head->next;
        Node* temp = copy;

        while (original && copy) {
            original->next = original->next ? original->next->next : original->next;
            copy->next = copy->next ? copy->next->next : copy->next;
            original = original->next;
            copy = copy->next;
        }

        return temp;
    }
    /*
    Time Complexity: O(N), where N is the number of nodes in the linked list.
    Space Complexity: O(N), where N is the number of nodes in the linked list.
    */
    Node* cloneLinkedList(Node* head) {

        unordered_map<Node*, Node*> mp;
        Node* curr = head;

        // Traverse original linked list to store new
        // nodes corresponding to original linked list
        while (curr != NULL) {
            mp[curr] = new Node(curr->data);
            curr = curr->next;
        }

        curr = head;

        // Loop to update the next and random pointers
        // of new nodes
        while (curr != NULL) {
            Node* newNode = mp[curr];
            newNode->next = mp[curr->next];
            newNode->random = mp[curr->random];
            curr = curr->next;
        }

        return mp[head];
    }

    void printList(Node* head) {
        while (head != NULL) {
            cout << head->data << "(";
            if (head->random)
                cout << head->random->data << ")";
            else
                cout << "null" << ")";

            if (head->next != NULL)
                cout << " -> ";
            head = head->next;
        }
        cout << endl;
    }
};
int main() {

    // Creating a linked list with random pointer
    Solution s;
    Node* head = new Node(1);
    head->next = new Node(2);
    head->next->next = new Node(3);
    head->next->next->next = new Node(4);
    head->next->next->next->next = new Node(5);
    head->random = head->next->next;
    head->next->random = head;
    head->next->next->random = head->next->next->next->next;
    head->next->next->next->random = head->next->next;
    head->next->next->next->next->random = head->next;

    // Print the original list
    cout << "Original linked list:\n";
    s.printList(head);

    Node* clonedList = s.cloneLinkedList(head);

    cout << "Cloned linked list:\n";
    s.printList(clonedList);

    return 0;
}