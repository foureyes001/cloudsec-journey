// 15. 3Sum  |  https://leetcode.com/problems/3sum/
// Pattern: Two Pointers (sort, fix one element, converging scan on the rest)
// Time: O(n^2)   Space: O(1) extra (excluding the output and sort)

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    vector<vector<int>> threeSum(vector<int>& nums) {
        sort(nums.begin(), nums.end());
        vector<vector<int>> v;
        for (auto k = nums.begin(); k != nums.end(); k++) {
            if (k > nums.begin() && *k == *(k - 1)) continue;   // skip duplicate fixed value
            auto i = k + 1;
            auto j = nums.end() - 1;
            while (i < j) {
                if (*k + *i + *j < 0) {
                    i++;
                } else if (*k + *i + *j > 0) {
                    j--;
                } else {
                    v.push_back({*k, *i, *j});
                    i++; j--;
                    while (i < j && *i == *(i - 1)) i++;   // skip i's duplicates (AFTER a match)
                    while (i < j && *j == *(j + 1)) j--;   // skip j's duplicates (AFTER a match)
                }
            }
        }
        return v;
    }
};

int main() {
    Solution s;
    vector<int> t1 = {-1,0,1,2,-1,-4};
    auto r = s.threeSum(t1);
    for (auto& t : r) { cout << "["; for (int x : t) cout << x << " "; cout << "] "; }
    cout << "\n";   // expect [-1 -1 2] [-1 0 1]

    vector<int> t2 = {0,0,0};
    r = s.threeSum(t2);
    for (auto& t : r) { cout << "["; for (int x : t) cout << x << " "; cout << "] "; }
    cout << "\n";   // expect [0 0 0]

    vector<int> t3 = {-4,-2,-2,-2,0,1,2,2,2,3,3,4,4,6,6};
    r = s.threeSum(t3);
    for (auto& t : r) { cout << "["; for (int x : t) cout << x << " "; cout << "] "; }
    cout << "\n";   // the LeetCode failing input
}