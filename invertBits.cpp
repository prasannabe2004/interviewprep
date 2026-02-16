#include <iostream>

using namespace std;

/*
Given a number, invert all bits of it.


n   101010101

*/

class Solution {
public:
    int invertBits(int num) {
        int bits = 0;
        int n = num;
        while(n) {
            bits++;
            n = n >> 1;
        }
        cout << bits << endl;
        return num ^ (1 << bits)-1;
    }
};

int main()
{
    Solution s;
    // input dec  1365
    // input bin  10101010101
    
    // output bin 01010101010
    // output dec 682

    cout << s.invertBits(1365) <<  endl;
    return 0;
}