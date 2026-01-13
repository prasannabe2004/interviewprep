#include <iostream>
#include <string>

using namespace std;

string removeOneDigitToMin(string num) {
    int n = num.length();
    if (n <= 1)
        return "0";

    bool removed = false;
    for (int i = 0; i < n - 1; ++i) {
        // Greedy choice: Remove the first digit that is larger than the next
        if (num[i] > num[i + 1]) {
            num.erase(i, 1);
            removed = true;
            break;
        }
    }

    // If no digit was removed (all digits were non-decreasing), remove the last one
    if (!removed) {
        num.pop_back();
    }

    // Handle leading zeros (e.g., "102" -> "02" -> "2")
    size_t first_non_zero = num.find_first_not_of('0');
    if (first_non_zero == string::npos) {
        return "0"; // All zeros case
    }

    return num.substr(first_non_zero);
}

int main() {
    string number{"14356"};

    string result = removeOneDigitToMin(number);
    cout << "Minimum number after removing 1 digit: " << result << endl;

    return 0;
}