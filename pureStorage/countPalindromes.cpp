#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Solution {
  public:
    vector<string> result;
    int expand(string s, int left, int right) {
        cout << "Expanding from index " << left << " to " << right << endl;
        int n = s.length();
        int c = 0;
        while (left >= 0 && right < n && s[left] == s[right]) {
            result.push_back(s.substr(left, right - left + 1));
            c++;
            left--;
            right++;
        }
        return c;
    }
    int countPalindromes(string s) {
        int n = s.length();
        int count = 0;

        for (int i = 0; i < n; i++) {
            count += expand(s, i, i);     // odd-length
            count += expand(s, i, i + 1); // even-length
        }

        return count;
    }
};

int main() {
    Solution solution;
    string s = "ababa";
    int result = solution.countPalindromes(s);
    cout << "Number of palindromic substrings in \"" << s << "\": " << result << endl;
    for (const auto& palindrome : solution.result) {
        cout << palindrome << endl;
    }
    cout << solution.result.size() << endl;
    return 0;
}