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

    ListNode* middleNode(ListNode* head) {
        ListNode* slow = head;
        ListNode* fast = head;

        while (fast != nullptr && fast->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;
        }

        return slow;
    }
};

int main() {
    Solution solution;
    Solution::ListNode* head = new Solution::ListNode(1);
    head->next = new Solution::ListNode(2);
    head->next->next = new Solution::ListNode(3);
    head->next->next->next = new Solution::ListNode(4);
    head->next->next->next->next = new Solution::ListNode(5);

    Solution::ListNode* middle = solution.middleNode(head);
    cout << "Middle node value: " << middle->val << endl;

    // Clean up memory
    while (head != nullptr) {
        Solution::ListNode* temp = head;
        head = head->next;
        delete temp;
    }

    return 0;
}