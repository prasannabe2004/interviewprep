#include <iostream>

using namespace std;

class Solution {
  public:
    struct ListNode {
        int val;
        ListNode* next;
        ListNode(int x) : val(x), next(nullptr) {
        }
    };

    bool detectLoop(ListNode* head) {
        ListNode* slow = head;
        ListNode* fast = head;

        while (fast != nullptr && fast->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;

            if (slow == fast) {
                return true; // Entry point of the loop
            }
        }
        return false; // No loop detected
    }
    bool fixLoop(ListNode* head) {
        ListNode* slow = head;
        ListNode* fast = head;

        // Detect loop
        bool hasLoop = false;
        while (fast != nullptr && fast->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;

            if (slow == fast) {
                hasLoop = true;
                break;
            }
        }

        if (!hasLoop) {
            return false; // No loop detected
        }

        // Find entry point of the loop
        slow = head;
        while (slow != fast) {
            slow = slow->next;
            fast = fast->next;
        }
        slow->next = nullptr; // Break the loop
        return true;          // Entry point of the loop
    }
};

int main() {
    Solution solution;
    Solution::ListNode* head = new Solution::ListNode(1);
    head->next = new Solution::ListNode(2);
    head->next->next = new Solution::ListNode(3);
    head->next->next->next = new Solution::ListNode(4);
    head->next->next->next->next = head->next; // Creating a loop for testing

    bool hasLoop = solution.detectLoop(head);
    cout << "Linked List has loop: " << (hasLoop ? "Yes" : "No") << endl;

    if (hasLoop) {
        bool loopFixed = solution.fixLoop(head);
        if (loopFixed) {
            cout << "Loop fixed successfully." << endl;
        }
    }
    hasLoop = solution.detectLoop(head);
    cout << "Linked List has loop after fixing: " << (hasLoop ? "Yes" : "No") << endl;
    // Note: In a real scenario, we should free the allocated memory.
    // However, since there's a loop, we would need to handle that carefully.

    return 0;
}