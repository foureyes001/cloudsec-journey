// LeetCode 128 - Longest Consecutive Sequence
// https://leetcode.com/problems/longest-consecutive-sequence/
// Pattern: Arrays & Hashing (hash set + run-start scan)
// Time:  O(n) - each value is visited at most twice; only run-starts trigger the forward walk
// Space: O(n) - the hash set
// Approach: load all values into an unordered_set. A value is a "run start" only if its
//   predecessor (v-1) is absent. From each run start, walk v+1, v+2, ... while present,
//   counting the run length, and track the maximum. The run-start guard is what keeps it O(n)
//   (it stops every run from being re-walked from each of its members).

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
using namespace std;

class Solution {
public:
    int longestConsecutive(vector<int>& nums) {
        unordered_set<int> m;
        int best=0;
        for(int i:nums)
        m.insert(i);
        for(auto &it:m)
        {
            int c=1;
            int i=it;
            if(m.find(i-1)==m.end())
            {while(m.find(i+1)!=m.end())
            {
            c++;
            i++;
            }}
            best=max(c,best);
        }
        return best;
    }
};

int main(){
    Solution s;
    vector<int> t1={};                      cout << s.longestConsecutive(t1) << "\n"; // expected: 0
    vector<int> t2={1};                     cout << s.longestConsecutive(t2) << "\n"; // expected: 1
    vector<int> t3={1,1};                   cout << s.longestConsecutive(t3) << "\n"; // expected: 1  (duplicates)
    vector<int> t4={100,4,200,1,3,2};       cout << s.longestConsecutive(t4) << "\n"; // expected: 4  ({1,2,3,4})
    vector<int> t5={0,3,7,2,5,8,4,6,0,1};   cout << s.longestConsecutive(t5) << "\n"; // expected: 9  ({0..8})
    vector<int> t6={0,0,-1};                cout << s.longestConsecutive(t6) << "\n"; // expected: 2  (negatives)
    return 0;
}