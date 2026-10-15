/*
 * 207. Course Schedule (Medium)
 *
 * Pattern    : Graph - take courses in rounds by counting prerequisites
 *              (Kahn's algorithm / topological order). Any course that is
 *              never ready is waiting on a loop.
 * Contract   : numCourses courses, numbered 0..numCourses-1.
 *              prerequisites[i] = [a, b] means b must be taken before a.
 *              Return true iff every course can be finished (no loop).
 * Complexity : Time  O(numCourses + pairs) visits, plus erase(begin())
 *              shifting the list (fine at numCourses <= 2000).
 *              Space O(numCourses + pairs).
 * Verified   : NOT YET RUN
 * Journey    : ASSISTED (first problem of the "loop in prerequisites" shape).
 *              v1: one pass over the pairs with a true/false flag - depends
 *              on the order of the pairs ([[2,1],[1,0]] -> false, should be
 *              true); never missed a real loop, but 2,147 false alarms in
 *              50,000 random tests.
 *              v2: built the lists correctly, then walked from course 0 only
 *              (the #841 question, not this one) - 11,701 wrong of 20,000.
 *              Help taken: two standard methods explained (three-state walk;
 *              rounds with counts); switched to the rounds method.
 *              v3: count of prerequisites per course, start with every
 *              course at count 0, lower counts as courses are taken.
 *              Two slips fixed: `*it--` moved the iterator instead of lowering
 *              a count; the zero check read the course number instead of
 *              its count.
 *              Transfer check due: #210 Course Schedule II.
 * Residual   : The other standard method (three states: not started /
 *              in progress / finished) is not yet learned.
 *              erase(begin()) shifts the list on every removal (kept).
 *              `i<prerequisites.size()` compares int with an unsigned size
 *              (compiler warning; harmless here). Bug family
 *              "signed/unsigned mixing".
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
using namespace std;

// ======================== SOLUTION ========================
class Solution {
public:
    bool canFinish(int numCourses, vector<vector<int>>& prerequisites) {
        vector<int> prog(numCourses,0);
        vector<int> s;
        vector<vector<int>> v(numCourses);
        for(int i=0;i<prerequisites.size();i++)
        {
            v[prerequisites[i][1]].push_back(prerequisites[i][0]);
            prog[prerequisites[i][0]]++;
        }
        for(int i=0;i<numCourses;i++)
        {
            if(!prog[i])
            s.push_back(i);}
        int c=0;
        while(!s.empty())
        {
            auto it=v[*s.begin()].begin();
            while(it!=v[*s.begin()].end())
            {
                prog[*it]--;
                if(!prog[*it])
                s.push_back(*it);
                it++;
            }
            s.erase(s.begin());
            c++;
        }
        return c==numCourses;
    }
};
// ====================== END SOLUTION ======================

// Harness (not graded)

// Independent reference: three-state depth-first walk
// (0 = not started, 1 = in progress, 2 = finished).
static bool walk(int u, const vector<vector<int> >& next, vector<int>& state) {
    state[u] = 1;
    for (size_t k = 0; k < next[u].size(); k++) {
        int w = next[u][k];
        if (state[w] == 1) return false;                   // back on the current chain: loop
        if (state[w] == 0 && !walk(w, next, state)) return false;
    }
    state[u] = 2;
    return true;
}

static bool referenceSolve(int n, const vector<vector<int> >& pairs) {
    vector<vector<int> > next(n);
    for (size_t i = 0; i < pairs.size(); i++) next[pairs[i][1]].push_back(pairs[i][0]);
    vector<int> state(n, 0);
    for (int i = 0; i < n; i++)
        if (state[i] == 0 && !walk(i, next, state)) return false;
    return true;
}

static int failures = 0;

static void check(bool ok, const char* name) {
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) failures++;
}

int main() {
    Solution sol;  // one instance reused for every test

    // 1. Fixed cases
    struct Case { int n; vector<vector<int> > pairs; bool expected; const char* name; };
    vector<Case> cases = {
        { 2, {{1,0}},                         true,  "LC example 1: 0 before 1" },
        { 2, {{1,0},{0,1}},                   false, "LC example 2: 0 and 1 need each other" },
        { 1, {},                              true,  "one course, no pairs" },
        { 3, {},                              true,  "three courses, no pairs" },
        { 3, {{2,1},{1,0}},                   true,  "chain given in reverse order" },
        { 3, {{1,0},{2,1},{1,2}},             false, "loop 1<->2 not through course 0" },
        { 4, {{1,0},{2,0},{3,1},{3,2}},       true,  "diamond: 3 reached twice, no loop" },
        { 5, {{1,4},{2,4},{3,1},{3,2}},       true,  "course 0 in no pair" },
        { 1, {{0,0}},                         false, "course needs itself" },
    };
    for (size_t i = 0; i < cases.size(); i++) {
        vector<vector<int> > p = cases[i].pairs;
        check(sol.canFinish(cases[i].n, p) == cases[i].expected, cases[i].name);
    }

    // 2. Fuzz against the independent reference
    {
        mt19937 rng(207);
        int mismatches = 0;
        for (int t = 0; t < 20000; t++) {
            int n = 1 + (int)(rng() % 7);
            int m = (int)(rng() % 10);
            vector<vector<int> > seen(n, vector<int>(n, 0));
            vector<vector<int> > p;
            for (int k = 0; k < m; k++) {
                int a = (int)(rng() % n), b = (int)(rng() % n);
                if (seen[a][b]) continue;  // LeetCode pairs are unique
                seen[a][b] = 1;
                vector<int> pair;
                pair.push_back(a);
                pair.push_back(b);
                p.push_back(pair);
            }
            vector<vector<int> > copyP = p;
            if (sol.canFinish(n, p) != referenceSolve(n, copyP)) mismatches++;
        }
        check(mismatches == 0, "fuzz 20,000 vs three-state reference (reused instance)");
    }

    // 3. Size limits: 2000 courses, 5000 pairs
    {
        int n = 2000;
        vector<vector<int> > p;
        for (int i = 1; i < n && p.size() < 5000; i++)
            for (int d = 1; d <= 3 && i - d >= 0 && p.size() < 5000; d++) {
                vector<int> pair;
                pair.push_back(i);
                pair.push_back(i - d);
                p.push_back(pair);
            }
        chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
        bool got = sol.canFinish(n, p);
        chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        check(got == true,  "2000 courses, 5000 pairs: correct");
        check(ms < 1000.0,  "2000 courses, 5000 pairs: under 1 s");
    }

    cout << (failures == 0 ? "\nALL PASS\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
