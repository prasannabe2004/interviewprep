#include <string>
using namespace std;

int countPalindromes(string s) {
    int n = s.length();
    int count = 0;

    auto expand = [&](int left, int right) {
        int c = 0;
        while (left >= 0 && right < n && s[left] == s[right]) {
            c++;
            left--;
            right++;
        }
        return c;
    };

    for (int i = 0; i < n; i++) {
        count += expand(i, i);     // odd-length
        count += expand(i, i + 1); // even-length
    }

    return count;
}