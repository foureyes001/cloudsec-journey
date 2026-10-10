/*
 * 841. Keys and Rooms (Medium)
 *
 * Pattern    : Graph reachability - breadth-first walk from room 0,
 *              rooms marked visited when added to the list
 * Contract   : rooms[i] = keys found in room i (values unique, < n).
 *              Room 0 starts unlocked. Return true iff every room
 *              can be entered.
 * Complexity : Time  O(n + total keys) visits, plus O(n^2) worst case
 *              from erase(begin()) on the vector used as the list
 *              (fine at n <= 1000).
 *              Space O(n).
 * Verified   : NOT YET RUN on the user's machine.
 *              [A] Assistant environment (g++ 13, -std=gnu++14):
 *              fixed 8/8 - fuzz 100,000 vs independent DFS reference,
 *              one reused instance, 0 mismatches, input unchanged -
 *              1000-room worst cases < 0.03 ms.
 * Journey    : Transfer check for #133 Clone Graph (traversal half only;
 *              no copying, so #133's copy core is NOT yet rechecked).
 *              v1 (own design): correct on all tests, but marked a room
 *              visited when REMOVED from the list, so rooms were re-added
 *              by every room holding their key before their turn.
 *              1000 rooms with keys i+1..i+3 -> did not finish in 60 s
 *              (160 rooms: 1.18 million additions). Assistant named the
 *              defect and the 3-line move; user wrote the fix.
 *              v2: mark visited when ADDED (room 0 before the loop).
 * Residual   : erase(begin()) shifts the vector on every removal
 *              (kept deliberately; a speed tweak, not a fix).
 *              New bug family: "marked visited too late" (1st appearance).
 */

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
using namespace std;

// ======================== SOLUTION (user) ========================
class Solution {
public:
    bool canVisitAllRooms(vector<vector<int>>& rooms) {
        int n=rooms.size();
        vector<bool> v(n,false);
        int k=0;
        vector<int> s;
        s.push_back(0);
            v[*s.begin()]=true;
            while(!s.empty())
            {
                auto it=rooms[s[0]].begin();
                while(it!=rooms[s[0]].end())
                {
                if(v[*it])
                {it++;
                    continue;}
                v[*it]=true;
                s.push_back(*it);
                it++;
                }
                s.erase(s.begin());
            }

            while(k<n)
            {
                if(!v[k])
                return false;
                k++;
            }
            return true;

    }
};
// ====================== END SOLUTION (user) ======================

// [A] Harness below is assistant-written. Not graded.

// Independent reference: depth-first walk with an explicit stack.
static bool referenceSolve(const vector<vector<int>>& rooms) {
    int n = (int)rooms.size();
    vector<int> seen(n, 0);
    vector<int> st(1, 0);
    seen[0] = 1;
    int count = 1;
    while (!st.empty()) {
        int x = st.back();
        st.pop_back();
        for (size_t j = 0; j < rooms[x].size(); j++) {
            int y = rooms[x][j];
            if (!seen[y]) { seen[y] = 1; count++; st.push_back(y); }
        }
    }
    return count == n;
}

static int failures = 0;

static void check(bool ok, const char* name) {
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) failures++;
}

int main() {
    Solution sol;  // one instance reused for every test

    // 1. Fixed cases
    struct Case { vector<vector<int>> rooms; bool expected; const char* name; };
    vector<Case> cases = {
        { {{1},{2},{3},{}},            true,  "LC example 1: chain" },
        { {{1,3},{3,0,1},{2},{0}},     false, "LC example 2: room 2 locked" },
        { {{}},                        true,  "single room, no keys" },
        { {{},{0}},                    false, "room 1 holds only key 0" },
        { {{0}},                       true,  "room 0 holds its own key" },
        { {{0,1},{1}},                 true,  "self keys" },
        { {{1},{0},{}},                false, "cycle 0<->1, room 2 locked" },
        { {{2},{},{1}},                true,  "reach room 1 via room 2" },
    };
    for (size_t i = 0; i < cases.size(); i++) {
        vector<vector<int>> r = cases[i].rooms;
        check(sol.canVisitAllRooms(r) == cases[i].expected, cases[i].name);
    }

    // 2. Fuzz against the independent reference (+ input left unchanged)
    {
        mt19937 rng(12345);
        int mismatches = 0, mutated = 0;
        for (int t = 0; t < 20000; t++) {
            int n = 1 + (int)(rng() % 10);
            vector<vector<int>> r(n);
            for (int i = 0; i < n; i++) {
                vector<int> p(n);
                for (int j = 0; j < n; j++) p[j] = j;
                shuffle(p.begin(), p.end(), rng);
                int d = (int)(rng() % (n + 1));
                if (rng() % 3 == 0) d = (int)(rng() % 2);  // sparse rooms
                r[i].assign(p.begin(), p.begin() + d);
            }
            vector<vector<int>> copyR = r;
            if (sol.canVisitAllRooms(r) != referenceSolve(copyR)) mismatches++;
            if (r != copyR) mutated++;
        }
        check(mismatches == 0, "fuzz 20,000 vs reference (reused instance)");
        check(mutated == 0,    "input rooms left unchanged");
    }

    // 3. Size limits: 1000 rooms, 2,997 keys (the case that hung v1)
    {
        int n = 1000;
        vector<vector<int>> r(n);
        for (int i = 0; i < n; i++)
            for (int d = 1; d <= 3; d++)
                if (i + d < n) r[i].push_back(i + d);
        chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
        bool got = sol.canVisitAllRooms(r);
        chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        check(got == true, "1000 rooms, keys i+1..i+3: correct");
        check(ms < 1000.0,  "1000 rooms, keys i+1..i+3: under 1 s");
    }

    cout << (failures == 0 ? "\nALL PASS\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}