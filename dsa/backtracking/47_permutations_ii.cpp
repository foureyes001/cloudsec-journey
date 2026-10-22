/*
 * 47. Permutations II  (Medium)
 *
 * Pattern:    Backtracking with duplicates. Sort, then at each position skip
 *             nums[j] when it equals nums[j-1] and index j-1 is not in the
 *             current path. Equal values are then always used in index order,
 *             so each arrangement is built exactly once.
 *
 * Contract:   Return every unique permutation of nums, any order.
 *             1 <= nums.size() <= 8, -10 <= nums[i] <= 10. nums may be sorted
 *             in place.
 *
 * Complexity: Time  O(n! * n^2) worst case (n! leaves, n candidates per call,
 *                   each candidate checked with a linear scan of s).
 *             Space O(n) recursion depth plus the output (up to n! * n ints).
 *
 * Verified:   NOT YET RUN
 *
 * Journey:    Transfer check for #40 Combination Sum II (duplicate skip).
 *             The #40 same-level skip had been forgotten.
 *             Ideas considered first: store results in a set (valid and fast,
 *             but removes duplicates after building them); find() on the
 *             results (correct but ~1.9 s on 8 distinct numbers).
 *             Value-based check ("is a 1 already in the list") works for two
 *             equal numbers but fails for three: [1,1,1] gave [111] twice.
 *             Help taken: choice tree for [1,1,2] drawn; skip condition given
 *             as fill-in-the-blank (equal to i-1 AND i-1 not in path).
 *             v1 had five defects, named: `!j` inverted (read nums[-1]);
 *             no stop check; s declared inside combo (fresh each call); no
 *             "index already used" skip; missing n.pop_back().
 *             v2 Accepted (13 ms). v3: n and s passed by reference.
 *             Same logic as the standard solution, arrived at with help.
 *
 * Residual:   #40 dedupe check not passed cold: re-derive #40.
 *             Optional: vector<bool> instead of contains for O(1) lookups.
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

// ranges::contains is C++23. Fallback so the file builds on older compilers
// (MinGW 6.3 defaults to C++14). Skipped when the library already has it.
#ifndef __cpp_lib_ranges_contains
#  if __cplusplus >= 202002L
namespace std { namespace ranges {      // C++20: std::ranges exists, contains doesn't
    template <class R, class T>
    bool contains(const R& r, const T& x) {
        return find(r.begin(), r.end(), x) != r.end();
    }
} }
#  else
namespace ranges {
    template <class R, class T>
    bool contains(const R& r, const T& x) {
        return find(r.begin(), r.end(), x) != r.end();
    }
}
#  endif
#endif

// ============================ SOLUTION START ============================

class Solution {
public:
    void combo(vector<vector<int>> &v,vector<int>& nums,vector<int>& n,vector<int>& s)
    {

        int j=0;

        while(j<nums.size())
        {

            if(n.size()==nums.size())
            {v.push_back(n);
                return;
            }

            if((ranges::contains(s,j)) ||(j && nums[j]==nums[j-1] && !ranges::contains(s,j-1)))
            {
                j++;
                continue;
            }
            s.push_back(j);
            n.push_back(nums[j]);
            combo(v,nums,n,s);
            n.pop_back();
            s.pop_back();
            j++;
        }

    }
    vector<vector<int>> permuteUnique(vector<int>& nums) {
        vector<vector<int>> v;
        vector<int> n;
        vector<int> s;

        sort(nums.begin(),nums.end());
        combo(v,nums,n,s);
        return v;
    }
};

// ============================= SOLUTION END =============================

// ------------------------------ harness --------------------------------
//
// Checks, for each input:
//   1. no permutation appears twice
//   2. every result is a rearrangement of the input
//   3. the set of results equals an independent reference
//      (std::next_permutation over the sorted input)
// One Solution object is reused for every test.

static vector<vector<int> > reference(vector<int> a) {
    sort(a.begin(), a.end());
    vector<vector<int> > out;
    do out.push_back(a); while (next_permutation(a.begin(), a.end()));
    return out;
}

static bool check(Solution& sol, const char* name, const vector<int>& input, bool print) {
    vector<int> sortedIn = input;
    sort(sortedIn.begin(), sortedIn.end());

    vector<int> arg = input;
    vector<vector<int> > got = sol.permuteUnique(arg);

    bool ok = true;
    const char* why = "";

    set<vector<int> > seen(got.begin(), got.end());
    if (seen.size() != got.size()) { ok = false; why = " [duplicate]"; }

    for (size_t i = 0; ok && i < got.size(); ++i) {
        vector<int> p = got[i];
        sort(p.begin(), p.end());
        if (p != sortedIn) { ok = false; why = " [not a rearrangement]"; }
    }

    vector<vector<int> > ref = reference(input);
    vector<vector<int> > g = got;
    sort(g.begin(), g.end());
    if (ok && g != ref) { ok = false; why = " [wrong set]"; }

    if (print || !ok)
        cout << (ok ? "PASS  " : "FAIL  ") << name << why << "\n";
    return ok;
}

static vector<int> make(const int* a, int n) { return vector<int>(a, a + n); }

int main() {
    Solution sol;
    int pass = 0, total = 0;

    { int a[] = {1};                     total++; pass += check(sol, "single           [1]", make(a, 1), true); }
    { int a[] = {1, 1, 2};               total++; pass += check(sol, "example 1        [1,1,2]", make(a, 3), true); }
    { int a[] = {1, 2, 3};               total++; pass += check(sol, "example 2        [1,2,3]", make(a, 3), true); }
    { int a[] = {1, 1, 1};               total++; pass += check(sol, "three equal      [1,1,1]", make(a, 3), true); }
    { int a[] = {1, 1, 1, 2};            total++; pass += check(sol, "three + one      [1,1,1,2]", make(a, 4), true); }
    { int a[] = {2, 1, 1};               total++; pass += check(sol, "unsorted input   [2,1,1]", make(a, 3), true); }
    { int a[] = {-1, 0, -1, 10};         total++; pass += check(sol, "negatives        [-1,0,-1,10]", make(a, 4), true); }
    { int a[] = {3, 3, 3, 3, 3, 3, 3, 3}; total++; pass += check(sol, "max, all equal   n=8", make(a, 8), true); }
    { int a[] = {1, 2, 3, 4, 5, 6, 7, 8}; total++; pass += check(sol, "max, distinct    n=8", make(a, 8), true); }
    { int a[] = {1, 1, 2, 2, 3, 3, 4, 4}; total++; pass += check(sol, "max, pairs       n=8", make(a, 8), true); }

    // fuzz: random inputs, small value range so duplicates are common
    unsigned seed = 12345u;
    int fuzzBad = 0;
    const int FUZZ = 3000;
    for (int t = 0; t < FUZZ; ++t) {
        seed = seed * 1103515245u + 12345u;
        int n = 1 + (int)((seed >> 16) % 7);          // 1..7
        vector<int> a(n);
        for (int i = 0; i < n; ++i) {
            seed = seed * 1103515245u + 12345u;
            a[i] = (int)((seed >> 16) % 4) - 1;        // -1..2
        }
        if (!check(sol, "fuzz case", a, false)) fuzzBad++;
    }
    total++;
    if (fuzzBad == 0) { pass++; cout << "PASS  fuzz             " << FUZZ << " random inputs\n"; }
    else cout << "FAIL  fuzz             " << fuzzBad << " of " << FUZZ << " wrong\n";

    cout << "\n" << pass << "/" << total << " passed\n";
    return pass == total ? 0 : 1;
}
