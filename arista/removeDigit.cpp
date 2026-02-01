#include <iostream>
#include <string>

using namespace std;

/*
You are given a string number that represents a positive integer and a character digit.

Your task is to remove exactly one occurrence of the character digit from the string number. After
removing this digit, the remaining characters form a new number. You need to choose which occurrence
of digit to remove such that the resulting number is as large as possible.

Example 1:
Input: 123451
Output: 23451
Explanation:
*/
class Solution {
  public:
    /*
    Time Complexity: O(n)
    Space Complexity: O(1)
    */
    string removeDigit(string number, char digit) {
        // Initialize the result with the smallest possible value
        string maxResult = "0";

        // Get the length of the input number string
        int length = number.size();

        // Iterate through each character in the number string
        for (int i = 0; i < length; ++i) {
            char currentChar = number[i];
            string candidateNumber = number.substr(0, i) + number.substr(i + 1);

            // Check if the current character matches the target digit
            if (currentChar == digit) {
                // Create a new string by removing the current digit
                // Concatenate the substring before index i with the substring after index i

                cout << "i = " << i << " " << number.substr(0, i) << " " << number.substr(i + 1)
                     << endl;

                // Update the result if the new number is lexicographically larger
                if (maxResult < candidateNumber) {
                    maxResult = candidateNumber;
                }
            }
        }

        // Return the maximum number after removing one occurrence of the digit
        return maxResult;
    }
};

int main() {
    Solution solution;
    string n("123151");
    cout << solution.removeDigit(n, '1') << endl;
    return 0;
}