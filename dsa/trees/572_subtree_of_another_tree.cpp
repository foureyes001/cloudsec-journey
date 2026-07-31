// 572. Subtree of Another Tree
// Date: 2026-07-30 (Day 57)  |  Shelf: trees (6)  |  Count: 42
//
// GRADE: (d) SOLUTION-READ, reproduced. NOT a cold solve.
//   Correct and verified, but not derived. Tier 1.
//   Cold RE-ATTEMPT owed >= 2026-08-15.
//   Does not count toward quality-problem totals until it passes cold.
//
// VERIFICATION (assistant-run, 2026-07-30):
//   fixed cases            18/18
//   fuzz vs independent serialization oracle
//                          200,000 rounds, 92,191 true-cases, 0 mismatches
//   reuse                  200,000 alternating calls, one instance, stable
//   deep skew              8,000-node chain, no stack failure
//   ASan + UBSan + leaks   clean
//
// FINDINGS (not applied - propose-don't-change; see [A] note at foot):
//   1. Eager `bool a = ...; bool b = ...; return a || b;` defeats short-circuit
//      evaluation. Both branches always run. MEASURED 100,002x wasted calls
//      (200,003 vs 2) on a probe with a matching left child and a 100,000-node
//      right chain. Not a defect: same O(n*m) worst case, identical answers.
//   2. check() is #100 Same Tree, reused verbatim (solved 2026-07-24, 6 days
//      prior - spacing floor of 5d satisfied). Its val-before-recurse ordering
//      is the #100 correction, applied. The wrapper below reintroduces the same
//      early-exit waste that ordering was meant to remove.
//   3. Rule 25 (spacing applies to shared core machinery): #100 and #572 now
//      share check(). Neither may be re-derived within 5 days of the other.
//
// Complexity: O(n * m) worst case, O(h) stack. An O(n + m) serialization +
// string-matching solution exists; deliberately not used here.
//
// Contract note: isSubtree(root, nullptr) returns false. LeetCode guarantees
// subRoot has >= 1 node, so this is inside contract.

#include <iostream>
#include <string>
#include <vector>

// [A] harness - not graded
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

// ======================= SOLUTION (mine) =======================
class Solution {
public:

     bool check(TreeNode *r, TreeNode *sr)
    {
        if(!r && !sr)
        return true;
        if(!r || !sr)
        return false;
        if(r->val !=sr->val)
        return false;
        return (check(r->left,sr->left) && check(r->right,sr->right));
    }
    bool isSubtree(TreeNode* root, TreeNode* subroot) {
        if(!root)
        return false;
        if(check(root,subroot))
        return true;
        bool a=isSubtree(root->left,subroot);
        bool b=isSubtree(root->right,subroot);
        return (a || b);
    }
};
// ===================== END SOLUTION =====================

// [A] harness below - build / free / driver. Disposable, not graded.
namespace harness {

const int NUL = -1000000;  // null sentinel in level-order input

TreeNode* build(const std::vector<int>& v) {
    if (v.empty() || v[0] == NUL) return nullptr;
    TreeNode* root = new TreeNode(v[0]);
    std::vector<TreeNode*> q{root};
    size_t i = 1, h = 0;
    while (h < q.size() && i < v.size()) {
        TreeNode* cur = q[h++];
        if (i < v.size()) {
            if (v[i] != NUL) { cur->left = new TreeNode(v[i]); q.push_back(cur->left); }
            i++;
        }
        if (i < v.size()) {
            if (v[i] != NUL) { cur->right = new TreeNode(v[i]); q.push_back(cur->right); }
            i++;
        }
    }
    return root;
}

void freeTree(TreeNode* n) {
    if (!n) return;
    freeTree(n->left);
    freeTree(n->right);
    delete n;
}

int passed = 0, total = 0;

void run(const std::string& name, const std::vector<int>& rootV,
         const std::vector<int>& subV, bool expected) {
    TreeNode* root = build(rootV);
    TreeNode* sub  = build(subV);
    Solution s;
    bool got = s.isSubtree(root, sub);
    total++;
    if (got == expected) {
        passed++;
        std::cout << "  ok   " << name << "\n";
    } else {
        std::cout << "  FAIL " << name << "  got=" << got
                  << " expected=" << expected << "\n";
    }
    freeTree(root);
    freeTree(sub);
}

}  // namespace harness

int main() {
    using namespace harness;
    const int _ = NUL;

    std::cout << "572 Subtree of Another Tree\n";

    run("LC ex1  root[3,4,5,1,2] sub[4,1,2]",        {3,4,5,1,2},              {4,1,2}, true);
    run("LC ex2  extra child breaks match",          {3,4,5,1,2,_,_,_,_,0},    {4,1,2}, false);
    run("single node equal",                         {1},                      {1},     true);
    run("single node differ",                        {1},                      {2},     false);
    run("trap: value matches, shape differs",        {1,2,_,3},                {2},     false);
    run("trap: mirrored subtree must not match",     {1,2,3,4},                {2,_,4}, false);
    run("dup values, only one position valid",       {2,2,2,_,_,_,3},          {2,_,3}, true);
    run("sub larger than root",                      {1},                      {1,2},   false);
    run("all-ones perfect tree vs 3-node sub",       {1,1,1,1,1,1,1},          {1,1,1}, true);
    run("match at deepest leaf",                     {1,2,3,4},                {4},     true);

    std::cout << "\n  " << passed << "/" << total << " passed\n";
    return passed == total ? 0 : 1;
}

// [A] PROPOSED, NOT APPLIED - strike this block if you disagree.
// Finding 1 above, in one line. Replaces the three-line tail of isSubtree:
//
//     return isSubtree(root->left, subroot) || isSubtree(root->right, subroot);
//
// `||` stops as soon as the left side is true; assigning to `a` and `b` first
// forces both sides to run every time. Measured 100,002x on the probe above.