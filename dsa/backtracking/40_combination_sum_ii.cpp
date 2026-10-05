
// #40 Combination Sum II  |  Medium  |  backtracking
//
// Pattern:    Backtracking over DISTINCT values. Count each value in a map,
//             then at each distinct value choose how many copies to take
//             (0..count) and move to the next value. Each combination is
//             built exactly once, so no duplicates are generated.
// Contract:   1 <= candidates.length <= 100, 1 <= candidates[i] <= 50,
//             1 <= target <= 30. Each element used at most once. No duplicate
//             combinations. Any output order.
// Complexity: Exponential part optimal: each distinct combination is built
//             once. Sorted map + early stop (value > target - sum) cuts every
//             branch that cannot fit. Remaining extra cost: n is copied on
//             every call (passed by value).
// Verified:   PASS
//             [A] Assistant environment: harness below 13/13 PASS, plus 20,000
//             random cases vs a brute-force reference, 0 mismatches.
// Journey:    Assisted. Hint ladder reached the dry-run rung (count map ->
//             one decision per distinct value -> drawn tree). Versions: shared
//             iterator + value*k push + no overshoot stop -> correct with
//             unordered_map + accumulate -> n by reference with a single pop
//             (leaked copies) -> pop inside the loop (n and sum disagreed) ->
//             n by value, running sum, sorted map -> early stop first checked
//             value vs whole target, after the skip call (did not fire) ->
//             final: value > target - sum, checked first. LeetCode 58 ms -> 7 ms.
// Residual:   n by value (copy per call, user's choice) · `auto` parameter is standard only in C++20 (GCC
//             accepts it as an extension) · m[*i]++ inserts on purpose.


#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>

using namespace std;

// ============================ SOLUTION START ============================

class Solution {
public:
    void combo(vector<vector<int>> &v,vector<int> n,auto it,map<int,int> &m,int target,int sum)
    {
        if(it==m.end())
        return;
        if(target-sum-it->first<0)
        return;
        combo(v,n,next(it),m,target,sum);
        int k=1;
        while(k<=it->second)
        {
        n.push_back(it->first);
        sum+=it->first;
        if(sum==target)
        {
            v.push_back(n);
            return;
        }
        else if(sum>target)
        return;
        combo(v,n,next(it),m,target,sum);
        k++;
        }
    }
    vector<vector<int>> combinationSum2(vector<int>& candidates, int target) {
        vector<vector<int>> v;
        vector<int> n;
        map<int,int> m;
        auto i=candidates.begin();
        while(i!=candidates.end())
        {
            m[*i]++;
            i++;
        }
        auto it=m.begin();
        combo(v,n,it,m,target,0);
        return v;
    }
};

// ============================= SOLUTION END =============================

// [A] Harness below is assistant-written. It checks properties, not printed
// output: same set of combinations as a brute-force reference, no duplicates.

static vector<vector<int>> normalize(vector<vector<int>> a) {
    for (size_t i = 0; i < a.size(); ++i)
        sort(a[i].begin(), a[i].end());
    sort(a.begin(), a.end());
    return a;
}

// Independent reference: try every subset of positions, keep sums == target,
// then remove duplicates. Shares no machinery with the solution.
static vector<vector<int>> reference(const vector<int>& a, int target) {
    vector<vector<int>> r;
    size_t n = a.size();
    for (unsigned long long mask = 0; mask < (1ULL << n); ++mask) {
        int s = 0;
        vector<int> pick;
        for (size_t j = 0; j < n; ++j)
            if (mask >> j & 1ULL) { s += a[j]; pick.push_back(a[j]); }
        if (s == target) { sort(pick.begin(), pick.end()); r.push_back(pick); }
    }
    sort(r.begin(), r.end());
    r.erase(unique(r.begin(), r.end()), r.end());
    return r;
}

static bool hasDuplicates(const vector<vector<int>>& g) {
    for (size_t i = 1; i < g.size(); ++i)
        if (g[i] == g[i - 1]) return true;
    return false;
}

static Solution shared;   // one instance reused across every case

static bool checkExact(const char* name, vector<int> cands, int target,
                       vector<vector<int>> expected) {
    vector<int> copy = cands;
    vector<vector<int>> g = normalize(shared.combinationSum2(copy, target));
    vector<vector<int>> e = normalize(expected);
    bool ok = (g == e) && !hasDuplicates(g);
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) cout << "        got " << g.size() << " combos, expected " << e.size() << "\n";
    return ok;
}

static bool checkCount(const char* name, vector<int> cands, int target, size_t expected) {
    vector<int> copy = cands;
    vector<vector<int>> g = normalize(shared.combinationSum2(copy, target));
    bool sumsOk = true;
    for (size_t i = 0; i < g.size(); ++i) {
        int s = 0;
        for (size_t j = 0; j < g[i].size(); ++j) s += g[i][j];
        if (s != target) sumsOk = false;
    }
    bool ok = g.size() == expected && !hasDuplicates(g) && sumsOk;
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) cout << "        got " << g.size() << " combos, expected " << expected << "\n";
    return ok;
}

static bool checkRandom(const char* name, int cases) {
    unsigned long long seed = 12345;
    int bad = 0;
    for (int c = 0; c < cases; ++c) {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        size_t n = 1 + (seed >> 33) % 14;
        int maxv = 1 + (int)((seed >> 20) % 8);
        int target = 1 + (int)((seed >> 10) % 20);
        vector<int> a(n);
        for (size_t j = 0; j < n; ++j) {
            seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
            a[j] = 1 + (int)((seed >> 33) % maxv);
        }
        vector<int> copy = a;
        vector<vector<int>> g = normalize(shared.combinationSum2(copy, target));
        if (g != reference(a, target) || hasDuplicates(g)) ++bad;
    }
    bool ok = (bad == 0);
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) cout << "        " << bad << " mismatches\n";
    return ok;
}

int main() {
    int pass = 0, total = 0;

    total++; pass += checkExact("LeetCode example 1        [10,1,2,7,6,1,5] t8",
        {10, 1, 2, 7, 6, 1, 5}, 8, {{1, 1, 6}, {1, 2, 5}, {1, 7}, {2, 6}});
    total++; pass += checkExact("LeetCode example 2        [2,5,2,1,2] t5",
        {2, 5, 2, 1, 2}, 5, {{1, 2, 2}, {5}});
    total++; pass += checkExact("no solution               [2] t1",
        {2}, 1, {});
    total++; pass += checkExact("single exact              [7] t7",
        {7}, 7, {{7}});
    total++; pass += checkExact("leaked-copy case          [1,2,2] t3",
        {1, 2, 2}, 3, {{1, 2}});
    total++; pass += checkExact("copies must stay          [1,1] t2",
        {1, 1}, 2, {{1, 1}});
    total++; pass += checkExact("copies must stay          [1,1,2] t4",
        {1, 1, 2}, 4, {{1, 1, 2}});
    total++; pass += checkExact("element used only once    [3] t6",
        {3}, 6, {});
    total++; pass += checkExact("all values too big        [40,50] t30",
        {40, 50}, 30, {});
    total++; pass += checkCount("100 ones                  t30",
        vector<int>(100, 1), 30, 1);
    vector<int> wide;
    for (int x = 1; x <= 50; ++x) { wide.push_back(x); wide.push_back(x); }
    total++; pass += checkCount("1..50 each twice          t30",
        wide, 30, 1225);
    total++; pass += checkExact("contract upper bound      [50,30] t30",
        {50, 30}, 30, {{30}});
    total++; pass += checkRandom("20000 random vs reference", 20000);

    cout << "\n" << pass << "/" << total << " passed\n";
    return pass == total ? 0 : 1;
}