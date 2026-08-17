// 78. Subsets — LeetCode (Medium)
// Pattern: backtracking — binary include/exclude decision per index, recursion supplies the
//          runtime-decided nesting depth that a fixed loop nest cannot.
// Contract: sets(v, n, i, nums) explores every subset of nums[i..end-1] built on top of the
//           partial subset n, and appends each completed one to v.
//           Record-on-add: every non-empty subset has a unique HIGHEST-index element and is
//           recorded exactly once, at the call that adds it. The empty set is the {{}} seed.
// Complexity: time O(n * 2^n), space O(n * 2^n) output + O(n) recursion depth.
//             The output is 2^n subsets of up to n elements — no solution can beat that.
// Design note: `n` is passed BY VALUE on purpose. Each call owns its copy, so the include
//              branch cannot corrupt the exclude branch and no undo step is needed. The trade
//              is copying, which is free at n <= 10. `nums` is const& — it never changes and
//              copying it per call was pure waste.
// Verified: NOT YET — fixed cases below only. Full battery still owed.
// Journey: v1 was iterative, two nested while loops -> capped at subsets of size <= 2, missed
//          [1,2,3] on a 3-element input. That wall IS the problem: subsets of an n-element
//          array need n nested loops, and n isn't known until runtime. v2 moved to recursion
//          but pushed nums[i] BEFORE both recursive calls, making both of them the include
//          branch -> [2] unreachable, [1,2] duplicated. v3 moved the push between the calls.
//          ASSISTED (hints on the loop-depth wall, the helper signature, and the missing
//          exclude branch).
// Residual: i==nums.size() is signed-vs-unsigned. Safe (i >= 0 always). FOURTH appearance of
//           this family after #567, #102, and the v1 loop here — past the three-appearance
//           threshold, so this wants a standing pre-commit check, not another per-file note.

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// ==================== SOLUTION — replace this block when re-deriving ====================
class Solution {
public:
    void sets(vector<vector<int>> &v, vector<int> n, int i, const vector<int>& nums)
    {
        if(i==nums.size())
        return;
        sets(v,n,i+1,nums);          // exclude nums[i]
        n.push_back(nums[i]);
        v.push_back(n);              // record-on-add
        sets(v,n,i+1,nums);          // include nums[i]
    }
    vector<vector<int>> subsets(vector<int>& nums) {
       vector<vector<int>> v={{}};
       vector<int> n;
       sets(v,n,0,nums);
       return v;
    }
};
// =======================================================================================

// --- self-checking harness: output order is unconstrained, so both sides are canonicalised ---
static vector<vector<int>> canon(vector<vector<int>> a) {
    for (size_t i = 0; i < a.size(); i++) sort(a[i].begin(), a[i].end());
    sort(a.begin(), a.end());
    return a;
}
static void show(const vector<vector<int>>& a) {
    cout << "[";
    for (size_t i = 0; i < a.size(); i++) {
        cout << "[";
        for (size_t j = 0; j < a[i].size(); j++) { cout << a[i][j]; if (j + 1 < a[i].size()) cout << ","; }
        cout << "]"; if (i + 1 < a.size()) cout << ",";
    }
    cout << "]";
}

static int failures = 0;
static void check(const char* name, vector<int> nums, vector<vector<int>> want) {
    Solution s;
    vector<vector<int>> got = s.subsets(nums);
    if (canon(got) == canon(want)) {
        cout << "PASS  " << name << "\n";
    } else {
        failures++;
        cout << "FAIL  " << name << "\n      got  "; show(canon(got));
        cout << "\n      want "; show(canon(want)); cout << "\n";
    }
}

// size + distinctness only, for inputs too large to write out
static void checkShape(const char* name, int n) {
    vector<int> nums;
    for (int i = 0; i < n; i++) nums.push_back(i);
    Solution s;
    vector<vector<int>> got = canon(s.subsets(nums));
    size_t expected = (size_t)1 << n;
    bool sizeOk = (got.size() == expected);
    bool distinct = (adjacent_find(got.begin(), got.end()) == got.end());
    if (sizeOk && distinct) {
        cout << "PASS  " << name << " (" << got.size() << " subsets, all distinct)\n";
    } else {
        failures++;
        cout << "FAIL  " << name << " size=" << got.size() << " want=" << expected
             << " distinct=" << (distinct ? "yes" : "no") << "\n";
    }
}

int main() {
    check("single element  ", {1},         {{},{1}});
    check("two elements    ", {1,2},       {{},{1},{2},{1,2}});
    check("LC sample       ", {1,2,3},     {{},{1},{2},{3},{1,2},{1,3},{2,3},{1,2,3}});
    check("zero            ", {0},         {{},{0}});
    check("negatives       ", {-1,2},      {{},{-1},{2},{-1,2}});
    check("unsorted input  ", {3,1,2},     {{},{3},{1},{2},{3,1},{3,2},{1,2},{3,1,2}});
    check("descending      ", {9,5},       {{},{9},{5},{9,5}});

    checkShape("n=4             ", 4);
    checkShape("n=10 (LC max)   ", 10);

    cout << "\n" << (failures ? "RESULT: FAILURES\n" : "RESULT: all cases pass\n");
    return failures ? 1 : 0;
}