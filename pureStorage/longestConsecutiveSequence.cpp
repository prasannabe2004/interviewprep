#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

/*
Find the longest consecutive elements sequence.
Example 1:
Input: nums = [100,4,200,1,3,2]
Output: 4
Explanation: The longest consecutive elements sequence is [1, 2, 3, 4]. 
Therefore its length is 4.
*/

class Solution {
   public:
   /*
    Time complexity : O(n)
    Space complexity : O(n)
   */
    int longestConsecutive(vector<int>& nums) {
        int max = 0;
        unordered_set<int> set;
        for (int num : nums) {
            set.insert(num);
        }
        for (int num : nums) {
            if (!set.count(num - 1)) {
                int y = num + 1;
                while (set.count(y)) {
                    y++;
                }
                max = std::max(max, y - num);
            }
        }
        return max;
    }
    /*
    Time complexity : O(nlogn)
    Space complexity : O(1)
    */
    int longestConsecutiveSorting(vector<int>& nums) {
        sort(nums.begin(), nums.end());
        int max = 0;
        int count = 1;
        for (int i = 1; i < nums.size(); i++) {
            if (nums[i] == nums[i - 1] + 1) {
                count++;
            } else if (nums[i] == nums[i - 1]) {
                continue;
            } else {
                max = std::max(max, count);
                count = 1;
            }
        }
        return std::max(max, count);
    }
};

int main() {
    Solution s;
    vector<int> nums = {100, 4, 200, 1, 3, 2};
    cout << s.longestConsecutive(nums) << endl;
    cout << s.longestConsecutiveSorting(nums) << endl;
    return 0;
}
