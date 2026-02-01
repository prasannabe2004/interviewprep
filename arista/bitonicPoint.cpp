#include <iostream>
#include <vector>
using namespace std;

/*
Given an array arr[] of integers which is initially strictly increasing and then strictly
decreasing, the task is to find the bitonic point, that is the maximum value in the array.

Note: Bitonic Point is a point in bitonic sequence before which elements are strictly increasing and
after which elements are strictly decreasing.

Input: arr[] = [1, 2, 4, 5, 7, 8, 3]
Output: 8
Explanation: 8 is the maximum element in the array.

Input: arr[] = [10, 20, 30, 40, 50]
Output: 50
Explanation: 50 is the maximum element in the array.

Input: arr[] = [120, 100, 80, 20, 0]
Output: 120
Explanation: 120 is the maximum element in the array.

*/
class Solution {
  public:
    /*
    Linear approach
    Time Complexity: O(n)
    Space Complexity: O(1)
    */
    int findMaximumLinear(vector<int>& arr) {
        int result = arr[0];
        int n = arr.size();
        for (int i = 1; i < n; ++i) {
            if (arr[i] > arr[i - 1]) {
                result = arr[i];
            } else {
                break;
            }
        }
        return result;
    }
    /*
    Binary search approach
    Time Complexity: O(log(n))
    Space Complexity: O(1)
    */
    int findMaximum(vector<int>& arr) {
        int n = arr.size();

        int l = 0;
        int r = n - 1;

        while (l <= r) {
            int mid = l + (r - l) / 2;
            if (arr[mid] > arr[mid - 1] && arr[mid] > arr[mid + 1]) {
                return arr[mid];
            } else if (arr[mid] > arr[mid - 1]) {
                l = mid + 1;
            } else {
                r = mid - 1;
            }
        }
        return -1;
    }
};

int main() {
    Solution solution;
    vector<int> arr{1, 2, 4, 5, 7, 8, 3};
    cout << solution.findMaximum(arr) << endl;
    cout << solution.findMaximumLinear(arr) << endl;
    return 0;
}