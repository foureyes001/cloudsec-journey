// 102. Binary Tree Level Order Traversal — LeetCode (Medium)
// Pattern: DFS carrying depth DOWN as a value parameter -> one bucket per depth.
// Contract: level(node, v, t) appends node->val to v[t]; creates bucket t on first arrival.
//           Invariant: a node at depth t is only reached via a parent at t-1, which already
//           guaranteed v.size() >= t. So v.size()<=t means exactly v.size()==t, and push_back
//           lands at index t. Never out of bounds, never a gap.
// Verified: NOT YET — fixed cases below only. Full battery (fuzz vs BFS oracle, reuse, deep
//           skew, ASan/UBSan) still owed before this is logged as verified.
// Journey: v1 computed depth on the way UP via max(k,l) -> that measures subtree HEIGHT, not
//          level, so siblings at different depths collided into one bucket ([[1],[2],[4,3]]);
//          also called v.reverse(), which std::vector has no such member. v2 added the depth
//          parameter but passed it as int& -> shared state, so depth never descended back for
//          the right sibling. v3 = pass by VALUE + create-on-first-arrival. ASSISTED (hints on
//          the direction of the parameter, the reference trap, and the off-by-one).
// Residual: v.size()<=t is unsigned-vs-signed. Safe (t is never negative). Logged, not fixed —
//           same family as the #567 contract-reliance note.

#include <iostream>
#include <vector>
using namespace std;

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

// ==================== SOLUTION — replace this block when re-deriving ====================
class Solution {
public:
    void level(TreeNode *root,vector<vector<int>> &v,int t)
    {
        if(!root)
        return;
        if(v.size()<=t)
        v.push_back({root->val});
        else
        v[t].push_back(root->val);
        int u=t+1;
        level(root->left,v,u);
        level(root->right,v,u);
    }

    vector<vector<int>> levelOrder(TreeNode* root) {

        int t=0;
        vector<vector<int>> v;

        level(root,v,t);
        return v;
    }
};
// =======================================================================================

// --- self-checking harness: prints PASS/FAIL per case, no manual eyeballing ---
static TreeNode* nd(int v, TreeNode* l = nullptr, TreeNode* r = nullptr) {
    TreeNode* n = new TreeNode(v); n->left = l; n->right = r; return n;
}
static void freeTree(TreeNode* r) { if (!r) return; freeTree(r->left); freeTree(r->right); delete r; }

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
static void check(const char* name, TreeNode* root, const vector<vector<int>>& want) {
    Solution s;
    vector<vector<int>> got = s.levelOrder(root);
    if (got == want) {
        cout << "PASS  " << name << "\n";
    } else {
        failures++;
        cout << "FAIL  " << name << "\n      got  "; show(got);
        cout << "\n      want "; show(want); cout << "\n";
    }
    freeTree(root);
}

int main() {
    check("null root       ", nullptr,                                        {});
    check("single node     ", nd(1),                                          {{1}});
    check("LC sample       ", nd(3, nd(9), nd(20, nd(15), nd(7))),            {{3},{9,20},{15,7}});
    check("left deeper     ", nd(1, nd(2, nd(4)), nd(3)),                     {{1},{2,3},{4}});
    check("right deeper    ", nd(1, nd(2), nd(3, nullptr, nd(4))),            {{1},{2,3},{4}});
    check("left skew       ", nd(1, nd(2, nd(3, nd(4)))),                     {{1},{2},{3},{4}});
    check("perfect depth 3 ", nd(1, nd(2, nd(4), nd(5)), nd(3, nd(6), nd(7))),{{1},{2,3},{4,5,6,7}});
    check("duplicate vals  ", nd(1, nd(1), nd(1)),                            {{1},{1,1}});
    check("negatives       ", nd(-1, nd(-2), nd(-3)),                         {{-1},{-2,-3}});

    cout << "\n" << (failures ? "RESULT: FAILURES\n" : "RESULT: all cases pass\n");
    return failures ? 1 : 0;
}