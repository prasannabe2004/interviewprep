#include <algorithm>
#include <iostream>
#include <string>

using namespace std;
/*
Given a number N as string, find the smallest number that has same set of digits as N and is greater
than N. If N is the greatest possible number with its set of digits, then print "Not Possible".

Examples:

Input: N = "218765"
Output: "251678"
Explanation: The next number greater than 218765 with same set of digits is 251678.

Input: n = "1234"
Output: "1243"
Explanation: The next number greater than 1234 with same set of digits is 1243.

Input: n = "4321"
Output: "Not Possible"
Explanation: 4321 is the greatest number possible with same set of digits.
*/

class Solution {
  public:
    /*
    Time Complexity: O(d) where d is the number of digits in the input number
    Space Complexity: O(d) where d is the number of digits in the input number
    */
    int nextGreaterElement(int n) {
        // Convert number to string for digit manipulation
        string digits = to_string(n);
        int length = digits.size();

        // Step 1: Find the rightmost digit that is smaller than its next digit
        // This is the pivot point where we need to make a change
        int pivotIndex = length - 2;
        while (pivotIndex >= 0 && digits[pivotIndex] >= digits[pivotIndex + 1]) {
            pivotIndex--;
        }

        // If no such digit exists, the number is already the largest permutation
        if (pivotIndex < 0) {
            return -1;
        }

        // Step 2: Find the smallest digit to the right of pivot that is larger than pivot
        // This digit will be swapped with the pivot
        int swapIndex = length - 1;
        while (digits[pivotIndex] >= digits[swapIndex]) {
            swapIndex--;
        }

        // Step 3: Swap the pivot with the found digit
        swap(digits[pivotIndex], digits[swapIndex]);

        // Step 4: Reverse all digits after the pivot position to get the smallest permutation
        // This ensures we get the next greater element, not just any greater element
        reverse(digits.begin() + pivotIndex + 1, digits.end());

        // Convert back to number and check for integer overflow
        long result = stol(digits);

        // Return -1 if the result exceeds 32-bit integer range
        return result > INT_MAX ? -1 : result;
    }
    /*
    Time Complexity: O(n)
    Space Complexity: O(1)
    */
    string nextPermutation(string N) {
        if (N.length() == 1) {
            return "Not Possible";
        }

        // Start from the right most digit and find the first
        // digit that is smaller than the digit next to it.
        int i = 0;
        for (i = N.length() - 1; i > 0; i--) {
            if (N[i] > N[i - 1])
                break;
        }
        cout << "Pivot element i: " << i << " " << N[i] << endl;

        // If i is 0 that means elements are in decreasing order
        if (i == 0) {
            return "Not Possible";
        }

        for (int j = N.length() - 1; j >= i; j--) {
            if (N[i - 1] < N[j]) {
                // Swap the found smallest digit i.e. N[j]
                // with N[i-1]
                cout << "Swapping element " << N[i - 1] << " " << N[j] << endl;
                swap(N[i - 1], N[j]);
                cout << "Swapped string " << N << endl;
                break;
            }
        }

        // Reverse the digits after (i-1)
        reverse(N.begin() + i, N.end());

        return N;
    }
};

int main() {
    Solution solution;
    string n("218765");
    int num = 218765;
    string r = solution.nextPermutation(n);
    cout << "Next greater element: " << r << endl;

    int result = solution.nextGreaterElement(num);
    cout << "Next greater element: " << result << endl;
    return 0;
}