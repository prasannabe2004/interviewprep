#include <stdio.h>
#include <string.h>

/**
 * Removes one digit to make the number the smallest possible.
 * Works by finding the first "peak" (a digit followed by a smaller digit).
 */
void removeOneToMin(char* num) {
    int n = strlen(num);
    if (n <= 1) {
        num[0] = '0';
        num[1] = '\0';
        return;
    }

    int found = 0;
    for (int i = 0; i < n - 1; i++) {
        // Greedy choice: Remove the first digit followed by a smaller one
        if (num[i] > num[i + 1]) {
            // Shift remaining characters left to "remove" current digit
            memmove(&num[i], &num[i + 1], n - i);
            found = 1;
            break;
        }
    }

    // If no such digit found (all digits were non-decreasing), remove the last one
    if (!found) {
        num[n - 1] = '\0';
    }

    // Handle leading zeros (e.g., "102" becomes "02", should be "2")
    int start = 0;
    while (num[start] == '0' && num[start + 1] != '\0') {
        start++;
    }

    if (start > 0) {
        memmove(num, &num[start], strlen(&num[start]) + 1);
    }
}

int main() {
    char number[100] = "14356";

    removeOneToMin(number);

    printf("Minimum possible number: %s\n", number);

    return 0;
}