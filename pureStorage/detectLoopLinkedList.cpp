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

    ListNode* startOfLoop(ListNode* head) {
        ListNode* slow = head;
        ListNode* fast = head;

        // Detect loop
        while (fast != nullptr && fast->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;

            if (slow == fast) {
                break;
            }
        }

        if (fast == nullptr || fast->next == nullptr) {
            return nullptr; // No loop detected
        }

        slow = head;
        while (slow != fast) {
            slow = slow->next;
            fast = fast->next;
        }
        return slow; // Entry point of the loop
    }
    bool fixLoop(ListNode* head) {
        ListNode* slow = head;
        ListNode* fast = head;

        // Detect loop
        while (fast != nullptr && fast->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;

            if (slow == fast) {
                break;
            }
        }

        if (slow == fast) {
            slow = head;
            while (slow->next != fast->next) {
                slow = slow->next;
                fast = fast->next;
            }
            fast->next = nullptr;
            return true;
        } else {
            return false;
        }
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
        Solution::ListNode* start = solution.startOfLoop(head);
        if (start != nullptr) {
            cout << "Start of the loop " << start->val << endl;
        }
    }

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