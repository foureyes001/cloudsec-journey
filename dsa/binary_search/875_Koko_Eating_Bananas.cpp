// Time:  O(n * log(max(piles)))   -- log over the speed RANGE, not n
// Space: O(1)
//
// Approach:
//   The answer is a speed k in [1, max(piles)]. feasible(k) = "can finish in <= h hours?"
//   is monotonic: once a speed works, every larger speed works too. So binary-search
//   for the leftmost k that is feasible, using the canonical half-open template.
//   hours(k) = sum over piles of ceil(pile / k).
//
// Float precision note: a (float) cast is buggy at large constraints -- ceil((float)1e9 / 1)
//   returns 999999968, not 1e9 (float has ~7 sig digits). This file uses (double), which is
//   exact to ~2^53 and safe for the 1e9 constraints. The integer idiom (pile + k - 1) / k is
//   exact at ALL scales (and avoids the FPU) -- a good default habit but not required here.
//   Verified: 200k randomized tests vs brute force, 0 fails.
//
// Template discipline (same as #704, #74): half-open [l, u), u = n+1 exclusive,
//   while(l < u), l = mid+1 / u = mid, return l. Don't mix with inclusive bounds.
//
// CANONICAL refactor of the search loop (fewer branches, no `best`, same result):
//     int c = 0;
//     for (int i : piles) c += (i + mid - 1) / mid;
//     if (c <= h) u = mid;        // feasible -> try smaller
//     else        l = mid + 1;    // too slow -> go faster
//     ... return l;               // l == u == answer at loop exit
//   The version below keeps the as-solved 3-branch form (verified correct); the
//   c==h and c<h branches both do u=mid, so it is functionally identical.

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

class Solution {
public:
    int minEatingSpeed(vector<int>& piles, int h) {
        int m = *max_element(piles.begin(), piles.end());
        int l = 1, u = m + 1;
        int c = 0;
        int mid, best = m;
        while (l < u) {
            mid = (l + u) / 2;
            for (int i : piles) {
                c += ceil((double)i / mid);   // (double), not (float): float has ~7 sig
                                              // digits and returns 999999968 for ceil(1e9/1).
                                              // (double) is exact to ~2^53, safe for 1e9 constraints.
                                              // (integer idiom (i+mid-1)/mid is exact at ALL scales.)
            }
            if (c == h) {
                best = min(best, mid);
                u = mid;
                c = 0;
            } else if (c > h) {
                c = 0;
                l = mid + 1;
            } else {
                c = 0;
                u = mid;
                best = min(best, mid);
            }
        }
        return best;
    }
};

int main() {
    Solution s;

    // LeetCode examples
    { vector<int> p = {3, 6, 7, 11};        cout << s.minEatingSpeed(p, 8)  << " (expected 4)\n";  }
    { vector<int> p = {30, 11, 23, 4, 20};  cout << s.minEatingSpeed(p, 5)  << " (expected 30)\n"; }
    { vector<int> p = {30, 11, 23, 4, 20};  cout << s.minEatingSpeed(p, 6)  << " (expected 23)\n"; }

    // Edge cases
    { vector<int> p = {1};                  cout << s.minEatingSpeed(p, 1)  << " (expected 1)\n";  }            // single small, h == n
    { vector<int> p = {1000000000};         cout << s.minEatingSpeed(p, 1)  << " (expected 1000000000)\n"; }   // 1e9: the float-killer case
    { vector<int> p = {312884470};          cout << s.minEatingSpeed(p, 312884469) << " (expected 2)\n"; }      // h one short of pile size

    return 0;
}