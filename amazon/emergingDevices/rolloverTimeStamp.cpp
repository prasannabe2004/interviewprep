#include <iostream>
#include <vector>

using namespace std;

/*
Function to process incoming timestamps and handle rollover
Example: If the timestamp is a 5-bit counter (0-31), and we receive timestamps in the order:


Input:
0, 1, 2, ..., 29, 30, 31, 0, 1, 2, ...


Output = 0, 1, 2, ..., 29, 30, 31,
30 (0 + 1*30), 31 (1 + 1*30), 32 (2 + 1*30), ...

Explanation:
- The first 32 timestamps (0 to 31) are processed normally.
- When the timestamp rolls over back to 0, we detect that the new timestamp (0) is less than the
previous timestamp (31), indicating a rollover.
- We increment the multiplier to account for the rollover, and subsequent timestamps are unwrapped
by adding the multiplier times the range to the incoming timestamp, ensuring that we maintain a
correct sequence of timestamps even after multiple rollovers.
*/

std::vector<long long> storage;
static int previous = -1;
static long long multiplier = 0;

const int RANGE = 30;

void processTimestamp(int ts) {

    if (previous != -1 && ts < previous) {
        // rollover detected
        multiplier++;
    }

    long long unwrapped = ts + multiplier * RANGE;
    storage.push_back(unwrapped);

    previous = ts;
}

int main() {
    vector<int> timestamps = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                              20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                              // Simulating a rollover
                              0, 1, 2, 3, 4};

    for (int ts : timestamps) {
        processTimestamp(ts);
    }

    cout << "Processed Timestamps: ";
    for (int ts : storage) {
        cout << ts << " ";
    }
    cout << endl;

    return 0;
}