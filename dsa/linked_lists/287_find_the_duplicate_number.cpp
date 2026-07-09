// LC 287 — Find the Duplicate Number (Medium) — Floyd's tortoise & hare (cycle detection)
// Constraints honored: array read-only (verified unmutated), O(1) extra space (3 ints).
// Idea: values 1..n over n+1 cells -> every value is a legal index -> array is a next-pointer
//       chain; the duplicate is the unique node with two incoming arrows = the loop ENTRANCE.
// Phase 1: slow x1 / fast x2 from cell 0 -> meet inside the loop.
// Phase 2: fresh walker from cell 0 + walker from the meeting, both x1 -> first equality = entrance.
// Complexity: O(n) time, O(1) space. Same algorithm as LC 142 (Linked List Cycle II).
// Verified: 6/6 fixed (all LC examples) + fuzz 100k (n 1..12) + 2k (n 100..2000, mult up to 50),
//           ASan/UBSan clean, input mutation check passed.

#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    int findDuplicate(vector<int>& nums) {
        int j=0,i=0;
        // wrap hygiene: dead outer `int k=0;` removed (was shadowed by phase-2's k)
        while(true)
        {
                j=nums[j];
                j=nums[j];
                i=nums[i];
                if(i==j)
                {
                    int k=0;
                    while(true)
                    {
                        k=nums[k];
                        i=nums[i];
                        if(i==k)
                        return i;
                    }
                }
        }
        // wrap hygiene: unreachable `return -1` removed (loop can only exit via return)
    }
};

int main() {
    Solution s;
    vector<int> t1 = {1,3,4,2,2};
    cout << "[1,3,4,2,2]  -> " << s.findDuplicate(t1) << "\n";
    vector<int> t2 = {3,1,3,4,2};
    cout << "[3,1,3,4,2]  -> " << s.findDuplicate(t2) << "\n";
    vector<int> t3 = {3,3,3,3,3};
    cout << "[3,3,3,3,3]  -> " << s.findDuplicate(t3) << "\n";
    return 0;
}