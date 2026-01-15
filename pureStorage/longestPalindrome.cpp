#include <iostream>
#include <string>
using namespace std;

class Solution {
  public:
    int expandAroundCenter(const string& s, int left, int right) {
        while (left >= 0 && right < s.length() && s[left] == s[right]) {
            left--;
            right++;
        }
        return right - left - 1; // Length of the palindrome
    }

    string longestPalindrome(string s) {
        if (s.empty())
            return "";
        int start = 0, end = 0;
        for (int i = 0; i < s.length(); i++) {
            // Expand around center
            int len1 = expandAroundCenter(s, i, i);     // Odd length
            int len2 = expandAroundCenter(s, i, i + 1); // Even length
            int len = max(len1, len2);
            // Update start and end indices if a longer palindrome is found
            if (len > end - start + 1) {
                start = i - (len - 1) / 2;
                end = i + len / 2;
            }
        }
        return s.substr(start, end - start + 1);
    }
    bool isPalindrome(const string& str, int left, int right) {
        while (left < right) {
            if (str[left] != str[right])
                return false;
            left++;
            right--;
        }
        return true;
    }
    string longestPalindromeBruteForce(string s) {
        int n = s.length();
        if (n == 0)
            return "";
        int start = 0;
        int maxLength = 1;
        for (int i = 0; i < n; i++) {
            for (int j = i; j < n; j++) {
                if (isPalindrome(s, i, j)) {
                    if (j - i + 1 > maxLength) {
                        maxLength = j - i + 1;
                        start = i;
                    }
                }
            }
        }
        return s.substr(start, start + maxLength);
    }
};

int main() {
    Solution solution;
    string s = "babad";
    string result = solution.longestPalindrome(s);
    cout << "Longest palindromic substring in \"" << s << "\": " << result << endl;

    result = solution.longestPalindromeBruteForce(s);
    cout << "Longest palindromic substring (Brute Force) in \"" << s << "\": " << result << endl;
    return 0;
}