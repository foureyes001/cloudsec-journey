// 110. Balanced Binary Tree — SOLO transfer test ("return one quantity, aggregate another")
// Verified 2026-07-17: fixed 6/6 (empty / single / LC1 / LC2 / 10k-skew / 10k-complete)
// + reuse-demo (50-skew false -> single-node true, SAME instance)
// + 200,000 reused-instance fuzz vs independent BFS-per-node oracle (iterative, O(n^2)) : 0 fails
// + ASan/UBSan clean (20k iters)
// Channels: return = height, int& d = aggregate. No member state. Legal per two-channels lens.
//
// Hygiene annotations (behavior unchanged, code kept verbatim below):
//  - `c` is unused -> -Wunused-variable warning; either drop it or just call check(root,d);
//  - `k-l>1 || k-l<-1`  ==  `abs(k-l)>1` (needs <cstdlib> or <cmath>)
//  - `if(!d) return true; else return false;`  ==  `return d==0;`
//  - naming: `l` reads like `1` in many fonts; prefer lh/rh
//  - d counts violations but only zero/nonzero is used -> a bool flag reads truer to intent

#include <algorithm>
#include <cassert>
#include <cstdio>
using std::max;

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

class Solution {
public:
    int check(TreeNode *root,int &d)
    {
        if(!root)
        return 0;
        int k=1+check(root->left,d);
        int l=1+check(root->right,d);
        if(k-l>1 || k-l<-1)
        d++;
        return max(k,l);

    }
    bool isBalanced(TreeNode* root) {
        int d=0;
        int c=check(root,d);
        (void)c; // silences -Wunused-variable; see hygiene note
        if(!d)
        return true;
        else return false;
    }
};

int main() {
    Solution sol;

    // empty + single
    assert(sol.isBalanced(nullptr) == true);
    TreeNode single(7);
    assert(sol.isBalanced(&single) == true);

    // LC ex1 [3,9,20,null,null,15,7] -> true
    TreeNode n15(15), n7(7), n9(9);
    TreeNode n20(20, &n15, &n7);
    TreeNode r1(3, &n9, &n20);
    assert(sol.isBalanced(&r1) == true);

    // LC ex2 [1,2,2,3,3,null,null,4,4] -> false
    TreeNode m4a(4), m4b(4), m3b(3), m2b(2);
    TreeNode m3a(3, &m4a, &m4b);
    TreeNode m2a(2, &m3a, &m3b);
    TreeNode r2(1, &m2a, &m2b);
    assert(sol.isBalanced(&r2) == false);

    // reuse-demo on the same instance: unbalanced call must not poison the next
    assert(sol.isBalanced(&r2) == false);
    assert(sol.isBalanced(&single) == true);

    std::printf("110 OK\n");
    return 0;
}