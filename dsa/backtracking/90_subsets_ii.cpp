/*
 * 90. Subsets II  -  Medium
 *
 * Pattern:    Backtracking / include-exclude enumeration, with canonical
 *             ordering so a set can collapse duplicate subsets.
 *
 * Contract:   1 <= nums.size() <= 10
 *             -10 <= nums[i] <= 10
 *             Return the power set with NO duplicate subsets.
 *             Neither the order of subsets nor the order of elements
 *             within a subset is specified by the problem.
 *
 * Complexity: time  O(n^2 * 2^n)  -- 2^n recursion nodes; each set insert
 *                                    compares vectors of length <= n
 *             space O(n * 2^n)    -- the set holds up to 2^n vectors
 *
 * Verified:   NOT YET RUN.  Do not trust this line until the harness has
 *             been executed:
 *               g++ -Wall -Wextra 90_subsets_ii.cpp -o out
 *               .\out.exe
 *             Replace this block with the PASS count once it is green.
 *
 * Journey:
 *   v1  map<vector<int>,int> as the dedup container, nums NOT sorted.
 *       WRONG.  The key is the emitted sequence, not the multiset, so two
 *       different index sets that form the same subset survive as two keys.
 *       Counterexample nums = [1,2,1]:
 *           indices {0,1} emit [1,2]
 *           indices {1,2} emit [2,1]
 *       Two keys, one subset.  Output 7 entries against an answer of 6.
 *
 *   v2  sort(nums) before recursing; map replaced by set.
 *       Sorting makes index order agree with value order, so every multiset
 *       has exactly one emitted sequence and the container collapses it.
 *       CORRECT.
 *       The sort was an assistant hint.  This is an ASSISTED acquisition,
 *       not a cold pass.
 *
 * Residual:
 *   - nums is passed BY VALUE into gather(), so the array is copied at each
 *     of the 2^n nodes.  Should be const vector<int>&.
 *     2nd appearance of this exact residual (also flagged on #78).
 *   - int i compared against nums.size() (size_t).
 *     7th appearance of the signed/unsigned family.  Contract-reliance,
 *     not a defect: n <= 10.
 *   - The canonical solution needs no auxiliary container at all: skip equal
 *     siblings at each recursion level instead, giving O(n * 2^n) and no set.
 *     Worth deriving separately.  This file keeps the version that was built.
 */

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

// ============================ SOLUTION START ============================

class Solution {
public:

    void gather(set<vector<int>> &m, vector<int> n, vector<int> nums, int i)
    {
        if (i == nums.size())
            return;
        gather(m, n, nums, i + 1);
        n.push_back(nums[i]);
        m.insert(n);
        gather(m, n, nums, i + 1);
    }

    vector<vector<int>> subsetsWithDup(vector<int>& nums) {

        set<vector<int>> m;
        vector<int> n;
        sort(nums.begin(), nums.end());
        gather(m, n, nums, 0);
        auto it = m.begin();
        vector<vector<int>> v = {{}};
        while (it != m.end())
        {
            v.push_back(*it);
            it++;
        }
        return v;
    }
};

// ============================= SOLUTION END =============================

// ------------------------------ harness --------------------------------

static vector<vector<int>> normalize(vector<vector<int>> a) {
    for (size_t i = 0; i < a.size(); ++i)
        sort(a[i].begin(), a[i].end());
    sort(a.begin(), a.end());
    return a;
}

static void dump(const char* label, const vector<vector<int>>& a) {
    cout << "        " << label << " (" << a.size() << "): ";
    for (size_t i = 0; i < a.size(); ++i) {
        cout << "[";
        for (size_t j = 0; j < a[i].size(); ++j) {
            if (j) cout << ",";
            cout << a[i][j];
        }
        cout << "]";
    }
    cout << "\n";
}

static bool checkExact(const char* name, vector<int> nums,
                       vector<vector<int>> expected) {
    Solution s;
    vector<vector<int>> got = s.subsetsWithDup(nums);
    vector<vector<int>> g = normalize(got);
    vector<vector<int>> e = normalize(expected);
    bool ok = (g == e);
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) {
        dump("got     ", g);
        dump("expected", e);
    }
    return ok;
}

// Count-only check for cases too large to write out.
// Also asserts the output contains no duplicate subsets, which is the
// whole point of the problem.
static bool checkCount(const char* name, vector<int> nums, size_t expected) {
    Solution s;
    vector<vector<int>> got = s.subsetsWithDup(nums);
    vector<vector<int>> g = normalize(got);

    bool dup = false;
    for (size_t i = 1; i < g.size(); ++i)
        if (g[i] == g[i - 1]) dup = true;

    bool ok = (g.size() == expected) && !dup;
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) {
        cout << "        size " << g.size() << ", expected " << expected
             << (dup ? ", DUPLICATES PRESENT" : "") << "\n";
    }
    return ok;
}

int main() {
    int pass = 0, total = 0;

    total++; pass += checkExact("duplicate pair            [1,2,2]",
        vector<int>{1, 2, 2},
        vector<vector<int>>{{}, {1}, {2}, {2, 2}, {1, 2}, {1, 2, 2}});

    total++; pass += checkExact("single element            [0]",
        vector<int>{0},
        vector<vector<int>>{{}, {0}});

    total++; pass += checkExact("unsorted input            [1,2,1]",
        vector<int>{1, 2, 1},
        vector<vector<int>>{{}, {1}, {1, 1}, {2}, {1, 2}, {1, 1, 2}});

    total++; pass += checkExact("all identical             [2,2,2]",
        vector<int>{2, 2, 2},
        vector<vector<int>>{{}, {2}, {2, 2}, {2, 2, 2}});

    total++; pass += checkExact("all distinct              [1,2,3]",
        vector<int>{1, 2, 3},
        vector<vector<int>>{{}, {1}, {2}, {3}, {1, 2}, {1, 3}, {2, 3}, {1, 2, 3}});

    total++; pass += checkExact("negatives and zero        [-1,0,1]",
        vector<int>{-1, 0, 1},
        vector<vector<int>>{{}, {-1}, {0}, {1}, {-1, 0}, {-1, 1}, {0, 1}, {-1, 0, 1}});

    total++; pass += checkExact("two pairs                 [1,1,2,2]",
        vector<int>{1, 1, 2, 2},
        vector<vector<int>>{{}, {1}, {1, 1}, {2}, {2, 2},
                            {1, 2}, {1, 1, 2}, {1, 2, 2}, {1, 1, 2, 2}});

    // interleaved duplicates, unsorted: sorted -> [1,4,4,4,4]
    // distinct subsets = (0..4 fours) * (with or without the 1) = 5 * 2 = 10
    total++; pass += checkCount("interleaved dups          [4,4,4,1,4]",
        vector<int>{4, 4, 4, 1, 4}, 10);

    // max size, all distinct -> full power set 2^10
    total++; pass += checkCount("max size all distinct     n=10 distinct",
        vector<int>{-10, -8, -6, -4, -2, 1, 3, 5, 7, 9}, 1024);

    // max size, all identical -> 0..10 copies = 11
    total++; pass += checkCount("max size all identical    n=10 identical",
        vector<int>{7, 7, 7, 7, 7, 7, 7, 7, 7, 7}, 11);

    // boundary values from the contract
    total++; pass += checkExact("contract bounds           [-10,10]",
        vector<int>{-10, 10},
        vector<vector<int>>{{}, {-10}, {10}, {-10, 10}});

    cout << "\n" << pass << "/" << total << " passed\n";
    return pass == total ? 0 : 1;
}