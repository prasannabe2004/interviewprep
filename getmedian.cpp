#include <iomanip>
#include <iostream>
#include <vector>
using namespace std;

/*
Given a data stream arr[] where integers are read sequentially, Determine the median of the elements
encountered so far after each new integer is read.

There are two cases for median on the basis of data set size.

If the data set has an odd number then the middle one will be consider as median.
If the data set has an even number then there is no distinct middle value and the median will be the
arithmetic mean of the two middle values.
*/

class Solution {
  public:
    vector<double> getMedian(vector<int>& arr) {
        vector<double> result;
        int n = arr.size();
        for (int i = 0; i < n; ++i) {
            if (i == 0) {
                result.push_back(arr[i]);
            } else {
                        }
        }
    }
};
