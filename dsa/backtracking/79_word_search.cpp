// ============================================================================
// #79 Word Search  |  Medium  |  backtracking on a grid
//
// Pattern:    Backtracking DFS. Start from every cell matching word[0]; step to
//             a neighbour that matches the next letter and is not already on
//             the current path. A separate `marked` grid records the path;
//             every mark is undone after its recursive call returns.
// Contract:   1 <= m, n <= 6, 1 <= word.length <= 15, board and word are
//             English letters. Same cell may not be used twice in one word.
// Complexity: O(m*n * 3^L) time in the worst case (L = word length; each step
//             has at most 3 new directions). O(m*n + L) extra space for the
//             marked grid and the recursion stack.
// Verified:   PASSED
//             [A] Assistant environment: harness below PASS, including 100,000
//             random boards vs an independent reference, 0 mismatches.
// Journey:    Assisted. v1: worklist with one shared index; abandoned by the
//             user's own reasoning (one k cannot serve many paths). v2:
//             backtracking, cells blanked to ' '; three defects shown by
//             counterexamples: results read uninitialised, start cell not
//             marked, blanked cells never restored. v3: separate marked grid,
//             all three fixed. Help taken on the 2D vector declaration syntax.
// Residual:   all four directions are tried even after one succeeds (no early
//             return) · int k compared with word.length() (signed/unsigned) ·
//             n and m passed by reference for no reason (plain int is fine).
// ============================================================================

#include <iostream>
#include <vector>
#include <string>

using namespace std;

// ============================ SOLUTION START ============================

class Solution {
public:

    bool comp(vector<vector<char>>& board, string &word,vector<vector<bool>> &marked,int i,int j,int k,int &n,int &m)
    {
        if(k==word.length())
        return true;
        int p=i,q=j;
        bool b1,b2,b3,b4;
        b1=b2=b3=b4=false;
        if(p+1<n && board[p+1][q]==word[k] && !marked[p+1][q])
        {
            marked[i+1][j]=true;
            
            b1=comp(board,word,marked,i+1,j,k+1,n,m);
            marked[i+1][j]=false;
        }

        if(p>0 && board[p-1][q]==word[k] && !marked[p-1][q])
        {
            
            marked[i-1][j]=true;
            b2=comp(board,word,marked,i-1,j,k+1,n,m);
            marked[i-1][j]=false;
        }
        
        if(q>0 && board[p][q-1]==word[k]&& !marked[p][q-1])
        {
            
            marked[i][j-1]=true;
            b3=comp(board,word,marked,i,j-1,k+1,n,m);
            marked[i][j-1]=false;
        }
        
        if(q+1<m && board[p][q+1]==word[k] && !marked[p][q+1])
        {
            
            marked[i][j+1]=true;
            b4=comp(board,word,marked,i,j+1,k+1,n,m);
            marked[i][j+1]=false;
        }
        if(b1 || b2 || b3 || b4)
        return true;
        return false;
    }
    bool exist(vector<vector<char>>& board, string word) {
        int n=board.size();
        int m=board[0].size();
        vector<vector<bool>> marked(n, vector<bool>(m, false)); 
        bool check=false;
        for(int i =0;i<n;i++)
        {
            for(int j=0;j<m;j++)
            {
                if(board[i][j]==word[0])
                {
                    
                    marked[i][j]=true;
                    check=comp(board,word,marked,i,j,1,n,m);}
                    marked[i][j]=false;
                if(check)
                return true;
            }
        }
        return false;
        
    }
};

// ============================= SOLUTION END =============================

// [A] Harness below is assistant-written. It checks properties: the answer
// matches a hand-derived value or an independent reference, and the board is
// unchanged afterwards.

static vector<vector<char>> makeBoard(const vector<string>& rows) {
    vector<vector<char>> g;
    for (size_t r = 0; r < rows.size(); ++r)
        g.push_back(vector<char>(rows[r].begin(), rows[r].end()));
    return g;
}

// Independent reference: plain DFS that blanks and restores cells in place.
static bool refFrom(vector<vector<char>>& g, const string& w,
                    int r, int c, size_t k) {
    if (k == w.size()) return true;
    if (r < 0 || c < 0 || r >= (int)g.size() || c >= (int)g[0].size()) return false;
    if (g[r][c] != w[k]) return false;
    char saved = g[r][c];
    g[r][c] = '#';
    bool ok = refFrom(g, w, r + 1, c, k + 1) || refFrom(g, w, r - 1, c, k + 1) ||
              refFrom(g, w, r, c + 1, k + 1) || refFrom(g, w, r, c - 1, k + 1);
    g[r][c] = saved;
    return ok;
}

static bool reference(vector<vector<char>> g, const string& w) {
    for (size_t r = 0; r < g.size(); ++r)
        for (size_t c = 0; c < g[0].size(); ++c)
            if (refFrom(g, w, (int)r, (int)c, 0)) return true;
    return false;
}

static Solution shared;   // one instance reused across every case

static bool check(const char* name, const vector<string>& rows,
                  const string& word, bool expected) {
    vector<vector<char>> g = makeBoard(rows);
    vector<vector<char>> before = g;
    bool got = shared.exist(g, word);
    bool ok = (got == expected) && (g == before);
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) cout << "        expected " << expected << ", got " << got
                  << (g == before ? "" : ", board was changed") << "\n";
    return ok;
}

static bool checkRandom(const char* name, int cases) {
    unsigned long long seed = 2026;
    int bad = 0;
    for (int t = 0; t < cases; ++t) {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        int n = 1 + (int)((seed >> 33) % 4);
        int m = 1 + (int)((seed >> 40) % 4);
        int letters = 1 + (int)((seed >> 50) % 3);
        int len = 1 + (int)((seed >> 20) % 8);
        vector<vector<char>> g(n, vector<char>(m));
        for (int r = 0; r < n; ++r)
            for (int c = 0; c < m; ++c) {
                seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
                g[r][c] = (char)('A' + (seed >> 33) % letters);
            }
        string w;
        for (int i = 0; i < len; ++i) {
            seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
            w += (char)('A' + (seed >> 33) % letters);
        }
        vector<vector<char>> copy = g;
        bool got = shared.exist(copy, w);
        if (got != reference(g, w) || copy != g) ++bad;
    }
    bool ok = (bad == 0);
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) cout << "        " << bad << " mismatches\n";
    return ok;
}

int main() {
    int pass = 0, total = 0;
    vector<string> lc = {"ABCE", "SFCS", "ADEE"};

    total++; pass += check("LeetCode example 1   ABCCED",            lc, "ABCCED", true);
    total++; pass += check("LeetCode example 2   SEE",               lc, "SEE", true);
    total++; pass += check("LeetCode example 3   ABCB (reuse)",      lc, "ABCB", false);
    total++; pass += check("single cell match",                      {"A"}, "A", true);
    total++; pass += check("single cell no match",                   {"A"}, "B", false);
    total++; pass += check("start cell reused     AB / ABA",         {"AB"}, "ABA", false);
    total++; pass += check("diagonal not adjacent AB,CD / ABCD",     {"AB", "CD"}, "ABCD", false);
    total++; pass += check("failed branch restored AB,AA / AAB",     {"AB", "AA"}, "AAB", true);
    total++; pass += check("snake path            AB,CD / ABDC",     {"AB", "CD"}, "ABDC", true);
    total++; pass += check("word longer than board",                 {"AB"}, "ABAB", false);
    total++; pass += check("6x6 all A, word needs B (worst case)",
        {"AAAAAA", "AAAAAA", "AAAAAA", "AAAAAA", "AAAAAA", "AAAAAA"},
        "AAAAAAAAAAAAAAB", false);
    total++; pass += check("6x6 all A, 15 A's",
        {"AAAAAA", "AAAAAA", "AAAAAA", "AAAAAA", "AAAAAA", "AAAAAA"},
        "AAAAAAAAAAAAAAA", true);
    total++; pass += checkRandom("100000 random boards vs reference", 100000);

    cout << "\n" << pass << "/" << total << " passed\n";
    return pass == total ? 0 : 1;
}