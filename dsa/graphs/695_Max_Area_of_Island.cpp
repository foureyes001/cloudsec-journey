// LeetCode #695 - Max Area of Island (Medium)
//
// Pattern:    Flood fill on a grid. A set of (row, col) pairs is the worklist;
//             cells are zeroed in the grid as they are processed.
// Contract:   grid is m x n, m,n >= 1 (LeetCode: up to 50 x 50), cells 0 or 1.
//             Island = 1s connected up/down/left/right (no diagonals).
//             Returns the largest island's area, or 0 if there is no land.
//             MUTATES the input grid (every visited 1 becomes 0).
// Complexity: O(m*n*log(m*n)) time because of the std::set, O(m*n) space.
// Verified:   NOT YET RUN
// Journey:    [fill in: versions, hints, what the judge said]
// Residual:   - mutates the caller's grid
//             - grid[i][j]=0 after the while loop is redundant ((i,j) is
//               already zeroed when processed)
//             - no empty-grid guard; grid[0] is undefined for an empty grid
//               (constraints exclude it)
//             - a set where a stack or queue would do; costs a log factor
//
// [A] Wrapper (includes, reference, harness) is assistant-written.
//     The Solution class is the author's, unchanged.

#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <utility>
#include <algorithm>
#include <string>
using namespace std;

// ===== SOLUTION BEGIN =====
class Solution {
public:
    int maxAreaOfIsland(vector<vector<int>>& grid) {
        int n=grid.size();
        int m=grid[0].size();
        set<pair<int,int>> s;
        int best=0;
        for(int i=0;i<n;i++)
        {
            for(int j=0;j<m;j++)
            {
                int c=0;
                if(grid[i][j])
                {
                    
                    int  p=i,q=j;
                    s.insert({p,q});
                    while(!s.empty())
                    {
                        auto it=s.begin();
                        p=it->first;
                        q=it->second;
                        if(p+1<n && grid[p+1][q])
                        {
                            s.insert({p+1,q});
                            }
                        if(q+1<m && grid[p][q+1])
                        {
                            s.insert({p,q+1});
                        }
                        if(p-1>=0 && grid[p-1][q])
                        {
                            s.insert({p-1,q});
                            }
                        if(q-1>=0 && grid[p][q-1])
                        {
                            s.insert({p,q-1});
                        }
                        c++;
                        grid[p][q]=0;
                        s.erase(it);
                    }
                    grid[i][j]=0;
                    best=max(best,c);
                }
            }
        }
        return best;
        
    }
};
// ===== SOLUTION END =====

// ---------------------------------------------------------------------------
// Harness
// ---------------------------------------------------------------------------
typedef vector<vector<int> > Grid;

// Independent reference: BFS with a separate visited array, never touches
// the input. Shares no machinery with Solution (no set, no mutation).
static int referenceMaxArea(const Grid& g)
{
    int n = g.size();
    int m = g[0].size();
    vector<vector<char> > seen(n, vector<char>(m, 0));
    const int dr[4] = {1, -1, 0, 0};
    const int dc[4] = {0, 0, 1, -1};
    int best = 0;
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < m; c++) {
            if (!g[r][c] || seen[r][c]) continue;
            queue<pair<int,int> > q;
            q.push(make_pair(r, c));
            seen[r][c] = 1;
            int area = 0;
            while (!q.empty()) {
                pair<int,int> cur = q.front();
                q.pop();
                area++;
                for (int d = 0; d < 4; d++) {
                    int nr = cur.first + dr[d];
                    int nc = cur.second + dc[d];
                    if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
                    if (!g[nr][nc] || seen[nr][nc]) continue;
                    seen[nr][nc] = 1;
                    q.push(make_pair(nr, nc));
                }
            }
            best = max(best, area);
        }
    }
    return best;
}

static int runSolution(const Grid& g)
{
    Grid copy = g;              // Solution mutates its argument
    Solution sol;
    return sol.maxAreaOfIsland(copy);
}

static int failures = 0;
static int checks = 0;

// Checks Solution against the hand-derived expectation AND the reference,
// so a wrong reference cannot hide behind a wrong expectation.
static void check(const string& name, const Grid& g, int expected)
{
    checks++;
    int got = runSolution(g);
    int ref = referenceMaxArea(g);
    if (got != expected || ref != expected) {
        failures++;
        cout << "FAIL " << name << ": expected " << expected
             << ", solution " << got << ", reference " << ref << "\n";
    }
}

static Grid filled(int n, int m, int v)
{
    return Grid(n, vector<int>(m, v));
}

int main()
{
    // --- hand-derived cases ---
    check("no land 2x2", Grid{{0,0},{0,0}}, 0);
    check("single land cell", Grid{{1}}, 1);
    check("single water cell", Grid{{0}}, 0);
    check("whole grid is land 3x4", filled(3, 4, 1), 12);
    check("diagonal cells are NOT connected", Grid{{1,0},{0,1}}, 1);
    check("island on the border", Grid{{1,1,1},{0,0,0},{0,0,1}}, 3);
    check("ring: cells reachable from two sides", Grid{{1,1,1},{1,0,1},{1,1,1}}, 8);
    check("1xN row", Grid{{1,1,0,1}}, 2);
    check("Nx1 column", Grid{{1},{1},{0},{1}}, 2);
    check("two islands, larger second", Grid{{1,0,0,0},{0,0,1,1},{0,0,1,1}}, 4);
    check("LeetCode example 1",
          Grid{{0,0,1,0,0,0,0,1,0,0,0,0,0},
               {0,0,0,0,0,0,0,1,1,1,0,0,0},
               {0,1,1,0,1,0,0,0,0,0,0,0,0},
               {0,1,0,0,1,1,0,0,1,0,1,0,0},
               {0,1,0,0,1,1,0,0,1,1,1,0,0},
               {0,0,0,0,0,0,0,0,0,0,1,0,0},
               {0,0,0,0,0,0,0,1,1,1,0,0,0},
               {0,0,0,0,0,0,0,1,1,0,0,0,0}}, 6);

    // --- maximum-size grids (limits) ---
    check("50x50 all land", filled(50, 50, 1), 2500);
    check("50x50 all water", filled(50, 50, 0), 0);
    Grid checker = filled(50, 50, 0);
    for (int r = 0; r < 50; r++)
        for (int c = 0; c < 50; c++)
            checker[r][c] = (r + c) % 2;
    check("50x50 checkerboard (all islands size 1)", checker, 1);

    // --- randomised, against the independent reference ---
    unsigned long state = 12345UL;
    for (int t = 0; t < 3000; t++) {
        state = state * 1103515245UL + 12345UL;
        int n = 1 + (int)((state >> 16) % 8);
        state = state * 1103515245UL + 12345UL;
        int m = 1 + (int)((state >> 16) % 8);
        state = state * 1103515245UL + 12345UL;
        int density = 20 + (int)((state >> 16) % 70);   // 20..89 percent land
        Grid g = filled(n, m, 0);
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < m; c++) {
                state = state * 1103515245UL + 12345UL;
                g[r][c] = ((int)((state >> 16) % 100) < density) ? 1 : 0;
            }
        }
        check("random #" + to_string(t), g, referenceMaxArea(g));
    }

    if (failures == 0) {
        cout << "ALL PASS (" << checks << " checks)\n";
        return 0;
    }
    cout << failures << " FAILED of " << checks << " checks\n";
    return 1;
}