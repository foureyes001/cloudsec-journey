// Problem: Contains Duplicate
// LeetCode #217 — https://leetcode.com/problems/contains-duplicate/
// Pattern: Arrays + Hashing (set membership)
// Time: O(n) | Space: O(n)
// Approach: Iterate, check if number already in set; if yes return true, else insert.

#include <iostream>
#include <vector>
#include <unordered_set>
using namespace std;

class Solution {
public:
    bool containsDuplicate(vector<int>& nums) {
        unordered_set<int> s;
        s.reserve(nums.size());
        for (int i : nums) {
            auto it = s.find(i);
            if (it != s.end())
                return true;
            s.insert(i);
        }
        return false;
    }
};

int main() {
    Solution sol;

    cout << boolalpha;

    vector<int> a = {1, 2, 3, 1};
    cout << sol.containsDuplicate(a) << endl;  // expect true

    vector<int> b = {1, 2, 3, 4};
    cout << sol.containsDuplicate(b) << endl;  // expect false

    vector<int> c = {};
    cout << sol.containsDuplicate(c) << endl;  // expect false (empty)

    return 0;
}