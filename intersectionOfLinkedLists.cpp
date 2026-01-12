#include <iostream>
#include <unordered_set>

using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    ListNode(int x) : val(x), next(nullptr) {
    }
};

class Solution {
  public:
    /*
    Time Complexity: O(N + M), where N and M are the lengths of the two linked lists.
    Space Complexity: O(1)
    */
    ListNode* getIntersectionNode(ListNode* headA, ListNode* headB) {
        if (!headA || !headB)
            return nullptr;

        ListNode* ptrA = headA;
        ListNode* ptrB = headB;

        while (ptrA != ptrB) {
            ptrA = ptrA ? ptrA->next : headB;
            ptrB = ptrB ? ptrB->next : headA;
        }

        return ptrA;
    }
    /*
    Time Complexity: O(N * M), where N and M are the lengths of the two linked lists.
    Space Complexity: O(1)
    */
    ListNode* getIntersectionNodeBruteForce(ListNode* headA, ListNode* headB) {

        for (ListNode* currA = headA; currA != nullptr; currA = currA->next) {
            for (ListNode* currB = headB; currB != nullptr; currB = currB->next) {
                if (currA == currB) {
                    return currA;
                }
            }
        }
        return nullptr;
    }
    /*
    Time Complexity: O(N + M), where N and M are the lengths of the two linked lists.
    Space Complexity: O(N) or O(M), depending on which list is stored in the hash set.
    */
    ListNode* getIntersectionNodeUsingHashing(ListNode* headA, ListNode* headB) {

        unordered_set<ListNode*> nodesInB;
        for (ListNode* currB = headB; currB != nullptr; currB = currB->next) {
            nodesInB.insert(currB);
        }

        for (ListNode* currA = headA; currA != nullptr; currA = currA->next) {
            if (nodesInB.find(currA) != nodesInB.end()) {
                return currA;
            }
        }
        return nullptr;
    }
    /*
    Time Complexity: O(N + M), where N and M are the lengths of the two linked lists.
    Space Complexity: O(1)
    */
    ListNode* getIntersectionNodeUsingLengthDifference(ListNode* headA, ListNode* headB) {
        int lenA = 0, lenB = 0;
        ListNode* currA = headA;
        ListNode* currB = headB;

        while (currA) {
            lenA++;
            currA = currA->next;
        }

        while (currB) {
            lenB++;
            currB = currB->next;
        }

        currA = headA;
        currB = headB;

        if (lenA > lenB) {
            for (int i = 0; i < lenA - lenB; i++) {
                currA = currA->next;
            }
        } else {
            for (int i = 0; i < lenB - lenA; i++) {
                currB = currB->next;
            }
        }

        while (currA && currB) {
            if (currA == currB) {
                return currA;
            }
            currA = currA->next;
            currB = currB->next;
        }

        return nullptr;
    }
};

int main() {
    // Example usage:
    // Create two linked lists that intersect and test the getIntersectionNode function.

    // List A: 4 -> 1 -> 2
    //                      -> 8 -> 4 -> 5
    // List B: 5 -> 0 -> 1

    ListNode* common = new ListNode(8);
    common->next = new ListNode(4);
    common->next->next = new ListNode(5);

    ListNode* headA = new ListNode(4);
    headA->next = new ListNode(1);
    headA->next->next = new ListNode(2);
    headA->next->next->next = common;

    ListNode* headB = new ListNode(5);
    headB->next = new ListNode(0);
    headB->next->next = new ListNode(1);
    headB->next->next->next = common;
    Solution solution;
    ListNode* intersection = solution.getIntersectionNode(headA, headB);
    if (intersection) {
        cout << "Intersection at node with value: " << intersection->val << endl;
    } else {
        cout << "No intersection." << endl;
    }

    intersection = solution.getIntersectionNodeBruteForce(headA, headB);
    if (intersection) {
        cout << "Intersection at node with value (Brute Force): " << intersection->val << endl;
    } else {
        cout << "No intersection (Brute Force)." << endl;
    }
    intersection = solution.getIntersectionNodeUsingHashing(headA, headB);
    if (intersection) {
        cout << "Intersection at node with value (Using Hashing): " << intersection->val << endl;
    } else {
        cout << "No intersection (Using Hashing)." << endl;
    }
    return 0;
}
