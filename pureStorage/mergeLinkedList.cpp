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

    ListNode* mergeTwoListsRecursive(ListNode* l1, ListNode* l2) {
        if (!l1)
            return l2;
        if (!l2)
            return l1;

        if (l1->val < l2->val) {
            l1->next = mergeTwoListsRecursive(l1->next, l2);
            return l1;
        } else {
            l2->next = mergeTwoListsRecursive(l1, l2->next);
            return l2;
        }
    }
    ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
        ListNode dummy(0);
        ListNode* next = &dummy;

        while (l1 != nullptr && l2 != nullptr) {
            if (l1->val < l2->val) {
                next->next = l1;
                l1 = l1->next;
            } else {
                next->next = l2;
                l2 = l2->next;
            }
            next = next->next;
        }

        if (l1 != nullptr) {
            next->next = l1;
        } else {
            next->next = l2;
        }

        return dummy.next;
    }
};

int main() {
    Solution solution;
    Solution::ListNode* l1 = new Solution::ListNode(1);
    l1->next = new Solution::ListNode(3);
    l1->next->next = new Solution::ListNode(5);

    Solution::ListNode* l2 = new Solution::ListNode(2);
    l2->next = new Solution::ListNode(4);
    l2->next->next = new Solution::ListNode(6);

    Solution::ListNode* merged = solution.mergeTwoListsRecursive(l1, l2);

    cout << "Merged linked list: ";
    while (merged != nullptr) {
        cout << merged->val << " ";
        merged = merged->next;
    }
    cout << endl;

    // Clean up memory
    // (In a real application, you'd want to delete all nodes to avoid memory leaks)

    return 0;
}