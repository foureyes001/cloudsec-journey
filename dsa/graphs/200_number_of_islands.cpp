/*
  #200 Number of Islands — Medium

  Pattern     : Grid flood-fill. Outer double loop finds an unvisited land cell and
                counts one island; inner while-loop drains that island completely via
                an explicit stack before the scan resumes. Mark-on-push, not on-pop.

  Contract    : grid is non-empty and rectangular (grid[0].size() is read directly).
                Land connects 4-directionally only, never diagonally.
                THE GRID IS DESTROYED — visited cells are overwritten with '0'.
                Caller must not reuse the grid after the call.

  Complexity  : Time  O(m*n)  — every cell examined once; optimal, since correctness
                              requires reading every cell.
                Space O(m*n)  — worst case stack size on an all-land grid.
                              A queue (BFS) would reduce this to O(min(m,n)) since the
                              frontier is a wavefront; time is unchanged.

  Verified    : fixed 12/12 + shape 1/1 in VS Code, incl. U-shape, ring, checkerboard,
                diagonal-only, single row/column, and 300x300 all-land.
                Iterative, so no recursion-depth risk on the max-constraint case.

  Journey     : v1 — counted cells with no land to the right and none below. Not an
                     island count; {"11","10"} returns 2. Also read out of range when
                     i+1==m and j+1==n were both true. No local rule over a fixed
                     neighbourhood can work: connectivity can run arbitrarily far.
                v2 — considered counting a cell unless it touched an already-visited
                     one. Overcounts on merge: {"101","111"} counts (0,0) and (0,2)
                     separately, then discovers via row 1 they were one island, with
                     no way to un-count. Fixing that means union-find.
                v3 — flood-fill. Only defect was lower bounds written as > 0 instead
                     of >= 0, which skipped row 0 and column 0 entirely.

  Residual    : grid[0].size() assumes non-empty — contract-reliance, not a defect.
                Four near-identical neighbour blocks; a dr[]/dc[] direction array
                would collapse them to one loop. Style only.
*/

#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <utility>
using namespace std;

// ==================== SOLUTION — replace this block when re-deriving ====================
class Solution {
public:
    int numIslands(vector<vector<char>>& grid) {
        int c = 0;
        int m = grid.size();
        int n = grid[0].size();
        stack<pair<int,int>> s;

        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                if (grid[i][j] == '1') {
                    grid[i][j] = '0';
                    s.push({i, j});
                    c++;
                }
                while (!s.empty()) {
                    int k = s.top().first;
                    int l = s.top().second;
                    s.pop();

                    if (k - 1 >= 0 && grid[k-1][l] == '1') {
                        grid[k-1][l] = '0';
                        s.push({k-1, l});
                    }
                    if (k + 1 < m && grid[k+1][l] == '1') {
                        grid[k+1][l] = '0';
                        s.push({k+1, l});
                    }
                    if (l - 1 >= 0 && grid[k][l-1] == '1') {
                        grid[k][l-1] = '0';
                        s.push({k, l-1});
                    }
                    if (l + 1 < n && grid[k][l+1] == '1') {
                        grid[k][l+1] = '0';
                        s.push({k, l+1});
                    }
                }
            }
        }
        return c;
    }
};
// ==================== END SOLUTION ====================

static vector<vector<char>> makeGrid(const vector<string>& rows) {
    vector<vector<char>> g;
    for (size_t i = 0; i < rows.size(); i++)
        g.push_back(vector<char>(rows[i].begin(), rows[i].end()));
    return g;
}

static int failures = 0;

static void check(const char* name, vector<string> rows, int want) {
    vector<vector<char>> g = makeGrid(rows);
    Solution s;
    int got = s.numIslands(g);
    if (got == want) {
        cout << "PASS  " << name << "\n";
    } else {
        failures++;
        cout << "FAIL  " << name << "  got " << got << "  want " << want << "\n";
    }
}

// all-land square, large enough to expose recursion depth in a recursive solution
static void checkDeep(const char* name, int n) {
    vector<string> rows(n, string(n, '1'));
    vector<vector<char>> g = makeGrid(rows);
    Solution s;
    int got = s.numIslands(g);
    if (got == 1) {
        cout << "PASS  " << name << " (" << n << "x" << n << ")\n";
    } else {
        failures++;
        cout << "FAIL  " << name << "  got " << got << "  want 1\n";
    }
}

int main() {
    check("single land     ", {"1"}, 1);
    check("single water    ", {"0"}, 0);
    check("LC example 1    ", {"11110",
                               "11010",
                               "11000",
                               "00000"}, 1);
    check("LC example 2    ", {"11000",
                               "11000",
                               "00100",
                               "00011"}, 3);
    check("diagonal only   ", {"10",
                               "01"}, 2);
    check("checkerboard    ", {"101",
                               "010",
                               "101"}, 5);
    check("all land        ", {"111",
                               "111",
                               "111"}, 1);
    check("all water       ", {"000",
                               "000",
                               "000"}, 0);
    check("single row      ", {"10101"}, 3);
    check("single column   ", {"1","0","1"}, 2);
    check("U shape         ", {"111",
                               "001",
                               "111"}, 1);
    check("ring            ", {"111",
                               "101",
                               "111"}, 1);

    checkDeep("max constraint  ", 300);

    cout << "\n" << (failures ? "RESULT: FAILURES\n" : "RESULT: all cases pass\n");
    return failures ? 1 : 0;
}