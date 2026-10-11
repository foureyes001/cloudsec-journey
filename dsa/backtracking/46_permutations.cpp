/*
 * 46. Permutations (Medium)
 *
 * Pattern    : Backtracking - choose / explore / un-choose.
 *              Each level fills ONE position: loop over every number,
 *              skip the ones already used, push, recurse, pop.
 * Contract   : nums has 1..6 distinct integers. Return every ordering
 *              of nums (n! lists), any order.
 * Complexity : Time  O(n^2 * n!) - n! results, each level scans n
 *              numbers with a linear "already used?" check.
 *              Space O(n^2) recursion (depth n, n copied per call),
 *              excluding the output.
 * Verified   : NOT YET RUN
 * Journey    : ASSISTED (first problem of the "fill a position" shape).
 *              v1: reused the Subsets take/skip template and kept only
 *              full-size lists -> [1,2,3] only (order can never change).
 *              v2: loop over all numbers, skip used ones - but no undo
 *              after the recursive call, so the loop's own copy of n kept
 *              growing -> [1,2,3] four times.
 *              v3: pop after the call -> correct.
 *              Help taken: a worked example of filling positions one by
 *              one; the missing undo pointed out; a dry run and choice
 *              tree afterwards to understand why it works.
 *              Link noticed afterwards: same shape as #79 Word Search
 *              (try each option, skip used, mark, recurse, unmark).
 *              Transfer check due: #47 Permutations II.
 * Residual   : n passed by value (one copy per call; deliberate).
 *              ranges::contains is C++23; the helper below supplies it
 *              for older compilers so the Solution compiles unchanged.
 *              Bug family "state not undone on every exit path":
 *              3rd appearance (#40, #79, #46).
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <set>
using namespace std;

// ranges::contains for compilers older than C++20 (e.g. MinGW 6.3).
// On C++23 the standard library version is used instead.
#if __cplusplus < 202002L
namespace ranges {
    template <class Container, class T>
    bool contains(const Container& c, const T& value) {
        return std::find(c.begin(), c.end(), value) != c.end();
    }
}
#endif

// ======================== SOLUTION ========================
class Solution {
public:

    void add(vector<vector<int>> &v,vector<int> n,vector<int>& nums)
    {
        if(n.size()==nums.size())
        {
            v.push_back(n);
            return;
        }

        for(int j:nums)
        {
            if(!ranges::contains(n,j))
            {
                n.push_back(j);
                add(v,n,nums);
                n.pop_back();
                }
        }


    }
    vector<vector<int>> permute(vector<int>& nums) {
        vector<vector<int>> v;
        vector<int> n;
        add(v,n,nums);
        return v;
    }
};
// ====================== END SOLUTION ======================

// Harness (not graded)

// Independent reference: every ordering via std::next_permutation.
static set<vector<int>> referenceSet(vector<int> nums) {
    set<vector<int>> out;
    sort(nums.begin(), nums.end());
    do { out.insert(nums); } while (next_permutation(nums.begin(), nums.end()));
    return out;
}

static long long factorial(int k) {
    long long f = 1;
    for (int i = 2; i <= k; i++) f *= i;
    return f;
}

// Properties: exactly n! lists, no duplicates, each list is an ordering
// of nums, and the set matches the reference.
static bool matchesReference(const vector<int>& nums, const vector<vector<int>>& got) {
    if ((long long)got.size() != factorial((int)nums.size())) return false;
    set<vector<int>> gotSet(got.begin(), got.end());
    if (gotSet.size() != got.size()) return false;  // duplicates
    return gotSet == referenceSet(nums);
}

static int failures = 0;

static void check(bool ok, const char* name) {
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) failures++;
}

int main() {
    Solution sol;  // one instance reused for every test

    // 1. Fixed cases
    struct Case { vector<int> nums; const char* name; };
    vector<Case> cases = {
        { {1,2,3},          "LC example 1: [1,2,3] -> 6 lists" },
        { {0,1},            "LC example 2: [0,1] -> 2 lists" },
        { {1},              "LC example 3: single number" },
        { {-1,0,1},         "negative numbers" },
        { {5,4,6,2},        "4 numbers -> 24 lists" },
        { {-10,10,3,0,7,1}, "6 numbers (max size) -> 720 lists" },
    };
    for (size_t i = 0; i < cases.size(); i++) {
        vector<int> nums = cases[i].nums;
        check(matchesReference(cases[i].nums, sol.permute(nums)), cases[i].name);
    }

    // 2. Fuzz against the reference (+ input left unchanged)
    {
        mt19937 rng(2026);
        int mismatches = 0, mutated = 0;
        for (int t = 0; t < 3000; t++) {
            vector<int> pool;
            for (int x = -10; x <= 10; x++) pool.push_back(x);
            shuffle(pool.begin(), pool.end(), rng);
            int n = 1 + (int)(rng() % 6);
            vector<int> nums(pool.begin(), pool.begin() + n);
            vector<int> before = nums;
            vector<vector<int>> got = sol.permute(nums);
            if (!matchesReference(before, got)) mismatches++;
            if (nums != before) mutated++;
        }
        check(mismatches == 0, "fuzz 3,000 vs next_permutation (reused instance)");
        check(mutated == 0,    "input nums left unchanged");
    }

    cout << (failures == 0 ? "\nALL PASS\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}