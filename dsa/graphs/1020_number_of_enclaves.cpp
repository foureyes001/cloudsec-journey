/*
 * 1020. Number of Enclaves (Medium)
 *
 * Pattern    : Grid flood fill from the border. Wipe out every land cell
 *              reachable from the edge; whatever land is left can never
 *              walk off the grid, so count it.
 * Contract   : grid is m x n of 0 (sea) / 1 (land), 1 <= m, n <= 500.
 *              Return the number of land cells from which you cannot walk
 *              off the grid. grid is modified (land set to 0).
 * Complexity : Time  O(m*n) cell visits - each cell enters the list at
 *              most once - plus the cost of erase(begin()) shifting the
 *              list (small on a grid; 500x500 worst case ~27 ms).
 *              Space O(m*n) worst case for the list.
 * Verified   : NOT YET RUN
 * Journey    : Transfer check for #200 Number of Islands.
 *              Own approach kept growing more complex; one hint taken
 *              (clear the border-connected land first) -> ASSISTED.
 *              The flood fill itself (#200's skill) was written without
 *              help, so #200 counts as retained.
 *              v1: correct (20,000 random grids, 0 mismatches) but
 *              time limit exceeded - the first loop marked a cell when
 *              it was REMOVED from the list, so neighbours re-added it
 *              and copies multiplied (10x10 all land: ~2 s).
 *              The second loop already marked on add; v2 does the same
 *              in the first loop (plus the starting border cell).
 *              Transfer check due for this problem: #1254 Number of
 *              Closed Islands.
 * Residual   : The second flood is not needed - after the first loop,
 *              every remaining 1 is an enclave and could just be counted.
 *              erase(begin()) shifts the list on every removal (kept).
 *              Bug family "marked visited too late": 2nd appearance
 *              (#841, #1020).
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
using namespace std;

// ======================== SOLUTION ========================
class Solution {
public:
    int numEnclaves(vector<vector<int>>& grid) {
        vector<pair<int,int>> v;

        int c=0;
        int n=grid.size();
        int m=grid[0].size();
        for(int i=0;i<n;i++)
        {
            for(int j=0;j<m;j++)
            {

                if(grid[i][j] && (!i || !j || i+1==n || j+1==m))
                {
                    v.push_back({i,j});
                    int p=i,q=j;
                    grid[i][j]=0;
                    while(!v.empty())
                    {
                        auto it=v.begin();
                        p=it->first;
                        q=it->second;
                        if(p+1<n && grid[p+1][q])
                        {
                            v.push_back({p+1,q});
                            grid[p+1][q]=0;
                        }

                        if(q+1<m && grid[p][q+1])
                        {
                            v.push_back({p,q+1});
                            grid[p][q+1]=0;
                        }

                        if(p>0  && grid[p-1][q])
                        {
                            v.push_back({p-1,q});
                            grid[p-1][q]=0;

                        }
                        if(q>0  && grid[p][q-1])
                        {
                            v.push_back({p,q-1});
                            grid[p][q-1]=0;


                        }
                        v.erase(v.begin());

                    }
                }
            }
        }
        for(int i=0;i<n;i++)
        {
            for(int j=0;j<m;j++)
            {

                if(grid[i][j])
                {
                    v.push_back({i,j});
                    int p=i,q=j;
                    grid[i][j]=0;
                    while(!v.empty())
                    {
                        auto it=v.begin();
                        p=it->first;
                        q=it->second;
                        if(p+1<n && grid[p+1][q])
                        {
                            v.push_back({p+1,q});
                            grid[p+1][q]=0;
                        }

                        if(q+1<m && grid[p][q+1])
                        {
                            v.push_back({p,q+1});
                            grid[p][q+1]=0;
                        }

                        if(p>0  && grid[p-1][q])
                        {
                            v.push_back({p-1,q});
                            grid[p-1][q]=0;
                        }
                        if(q>0  && grid[p][q-1])
                        {
                            v.push_back({p,q-1});
                            grid[p][q-1]=0;
                        }
                        c++;
                        v.erase(v.begin());

                    }
                }
            }
        }
        return c;

    }
};
// ====================== END SOLUTION ======================

// Harness (not graded)

// Independent reference: depth-first walk with an explicit stack.
static int referenceSolve(vector<vector<int>> g) {
    int n = (int)g.size(), m = (int)g[0].size();
    vector<pair<int,int> > st;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (g[i][j] && (i == 0 || j == 0 || i == n - 1 || j == m - 1)) {
                g[i][j] = 0;
                st.push_back(make_pair(i, j));
            }
    int dr[4] = {1, -1, 0, 0}, dc[4] = {0, 0, 1, -1};
    while (!st.empty()) {
        pair<int,int> cur = st.back();
        st.pop_back();
        for (int k = 0; k < 4; k++) {
            int a = cur.first + dr[k], b = cur.second + dc[k];
            if (a >= 0 && b >= 0 && a < n && b < m && g[a][b]) {
                g[a][b] = 0;
                st.push_back(make_pair(a, b));
            }
        }
    }
    int count = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) count += g[i][j];
    return count;
}

static int failures = 0;

static void check(bool ok, const char* name) {
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) failures++;
}

int main() {
    Solution sol;  // one instance reused for every test

    // 1. Fixed cases
    struct Case { vector<vector<int> > grid; int expected; const char* name; };
    vector<Case> cases = {
        { {{0,0,0,0},{1,0,1,0},{0,1,1,0},{0,0,0,0}}, 3, "LC example 1 -> 3" },
        { {{0,1,1,0},{0,0,1,0},{0,0,1,0},{0,0,0,0}}, 0, "LC example 2 -> 0" },
        { {{1}},                                     0, "single land cell (on border)" },
        { {{0}},                                     0, "single sea cell" },
        { {{1,1,1},{1,1,1},{1,1,1}},                 0, "all land, joined to border" },
        { {{0,0,0},{0,1,0},{0,0,0}},                 1, "one enclosed cell" },
        { {{1,1,1,1,1}},                             0, "single row" },
        { {{0},{1},{0}},                             0, "single column" },
    };
    for (size_t i = 0; i < cases.size(); i++) {
        vector<vector<int> > g = cases[i].grid;
        check(sol.numEnclaves(g) == cases[i].expected, cases[i].name);
    }

    // 2. Fuzz against the independent reference
    {
        mt19937 rng(1020);
        int mismatches = 0;
        for (int t = 0; t < 20000; t++) {
            int n = 1 + (int)(rng() % 9), m = 1 + (int)(rng() % 9);
            int density = (int)(rng() % 100);
            vector<vector<int> > g(n, vector<int>(m));
            for (int i = 0; i < n; i++)
                for (int j = 0; j < m; j++)
                    g[i][j] = (int)(rng() % 100) < density ? 1 : 0;
            vector<vector<int> > copyG = g;
            if (sol.numEnclaves(g) != referenceSolve(copyG)) mismatches++;
        }
        check(mismatches == 0, "fuzz 20,000 grids vs reference (reused instance)");
    }

    // 3. Size limits: 500 x 500 (the case that timed out in v1)
    {
        int N = 500;
        vector<vector<int> > g(N, vector<int>(N, 1));
        for (int j = 0; j < N; j++) g[0][j] = g[N-1][j] = g[j][0] = g[j][N-1] = 0;
        chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
        int got = sol.numEnclaves(g);
        chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        check(got == 498 * 498, "500x500, land inside a sea ring: correct");
        check(ms < 1000.0,      "500x500, land inside a sea ring: under 1 s");
    }
    {
        int N = 500;
        vector<vector<int> > g(N, vector<int>(N, 1));
        chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
        int got = sol.numEnclaves(g);
        chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
        double ms = chrono::duration<double, milli>(t1 - t0).count();
        check(got == 0,    "500x500, all land: correct");
        check(ms < 1000.0, "500x500, all land: under 1 s");
    }

    cout << (failures == 0 ? "\nALL PASS\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}