#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
using namespace std;

struct Point {
    int x, y;
};

class Solution {

  public:
    // Function to calculate squared distance between two points
    long long distSq(Point p1, Point p2) {
        return (long long)(p1.x - p2.x) * (p1.x - p2.x) + (long long)(p1.y - p2.y) * (p1.y - p2.y);
    }

    bool isSquare(Point p1, Point p2, Point p3, Point p4) {
        // Calculate all 6 possible squared distances between the 4 points
        std::vector<long long> d = {distSq(p1, p2), distSq(p1, p3), distSq(p1, p4),
                                    distSq(p2, p3), distSq(p2, p4), distSq(p3, p4)};

        // Sort distances to easily identify sides and diagonals
        std::sort(d.begin(), d.end());

        for (auto dist : d) {
            cout << dist << " ";
        }
        cout << endl;
        // A square must have:
        // 1. Four equal sides (shortest 4 distances)
        // 2. Two equal diagonals (longest 2 distances)
        // 3. Non-zero sides (points are distinct)
        // 4. Diagonal squared = 2 * side squared (Pythagorean theorem)
        return d[0] > 0 &&                                     // Distinct points
               d[0] == d[1] && d[1] == d[2] && d[2] == d[3] && // 4 equal sides
               d[4] == d[5] &&                                 // 2 equal diagonals
               d[4] == 2 * d[0];                               // d^2 = 2 * s^2
    }
};

int main() {
    Solution solution;
    Point p1 = {0, 0};
    Point p2 = {1, 1};
    Point p3 = {1, 0};
    Point p4 = {0, 1};

    if (solution.isSquare(p1, p2, p3, p4)) {
        cout << "The points form a square." << endl;
    } else {
        cout << "The points do not form a square." << endl;
    }

    p1 = {0, 0};
    p2 = {1, 2};
    p3 = {2, 1};
    p4 = {0, 1};
    if (solution.isSquare(p1, p2, p3, p4)) {
        cout << "The points form a square." << endl;
    } else {
        cout << "The points do not form a square." << endl;
    }

    return 0;
}