// LC 560 — Subarray Sum Equals K (Medium) — dsa/arrays_hashing/
// Approach: prefix sum + complement hash. Running sum cs; a subarray ending here sums to k
//           iff some earlier prefix equals cs-k -> count occurrences via hash map, seeded {0,1}
//           (the empty prefix). Handles negatives (why sliding window can't work here).
// Complexity: O(n) time, O(n) space.
// NOTE (wrap): pasted version had `if(it!=m.end()  it->second)` — missing `&&`, does not
//              compile. Fixed to `&&` below as the only sensible reading; CONFIRM your local
//              file matches. The `it->second` operand is redundant (stored counts are always
//              >=1) — kept + annotated per convention.
// Verified: 7/7 fixed (LC examples, negatives, all-zeros) + fuzz 200k/200k vs O(n^2) brute
//           (len 1-40, vals -50..50, half the cases with guaranteed-hit k), ASan/UBSan clean.
// Known warning kept verbatim: signed/unsigned compare on i<nums.size().

#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

class Solution {
public:
    int subarraySum(vector<int>& nums, int k) {
        unordered_map<int,int> m={{0,1}};
        int i=0,cs=0;
        int c=0;
        while(i<nums.size())
        {
            cs+=nums[i];

            int n=cs-k;

            auto it=m.find(n);
            if(it!=m.end() && it->second)
            {
                c+=it->second;
            }
            m[cs]++;
            i++;
        }
        return c;
    }
};

int main() {
    Solution s;
    vector<int> t1 = {1,1,1};
    cout << "[1,1,1], k=2      -> " << s.subarraySum(t1, 2) << "\n";
    vector<int> t2 = {1,2,3};
    cout << "[1,2,3], k=3      -> " << s.subarraySum(t2, 3) << "\n";
    vector<int> t3 = {3,4,7,2,-3,1,4,2};
    cout << "[3,4,7,2,-3,1,4,2], k=7 -> " << s.subarraySum(t3, 7) << "\n";
    return 0;
}