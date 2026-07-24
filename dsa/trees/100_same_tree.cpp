// 100. Same Tree
// Pattern: Binary Tree / DFS (parallel traversal of two trees)
// Time:  O(n)   n = nodes in the smaller tree; every matched pair visited once
// Space: O(h)   recursion stack; h = height (O(n) on a skewed tree)
//
// Approach: recurse on the two trees in lockstep.
//   - both null                -> identical (base case)
//   - exactly one null         -> structures differ, reject
//   - both non-null            -> left subtrees must match, values must match,
//                                 right subtrees must match
// Handling "exactly one null" explicitly is what makes structure-difference
// detection sound: two trees with the same values but different shapes hit a
// null-vs-node pair at the point where they diverge.

#include <iostream>
#include <string>
#include <vector>
#include <deque>

using namespace std;

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
    bool isSameTree(TreeNode* p, TreeNode* q) {
        if (!q && !p)
            return true;
        else if ((!q && p) || (q && !p))
            return false;

        bool k = isSameTree(p->left, q->left);
        if (!k)
            return false;

        if (q->val != p->val)
            return false;

        bool l = isSameTree(p->right, q->right);
        if (!l)
            return false;

        return true;
    }
};

// --------------------------- local test harness ---------------------------

struct Pool {                          // node storage; deque keeps pointers stable
    deque<TreeNode> nodes;
    TreeNode* make(int v) { nodes.emplace_back(v); return &nodes.back(); }
};

int main() {
    Solution sol;
    Pool P;

    struct T { const char* name; TreeNode* a; TreeNode* b; bool exp; };
    vector<T> tests;

    tests.push_back({"both null", nullptr, nullptr, true});
    tests.push_back({"p null, q node", nullptr, P.make(1), false});
    tests.push_back({"p node, q null", P.make(1), nullptr, false});
    tests.push_back({"single equal", P.make(5), P.make(5), true});
    tests.push_back({"single differ", P.make(5), P.make(6), false});

    {   // same values, different shape: left-child vs right-child
        TreeNode* a = P.make(1); a->left  = P.make(2);
        TreeNode* b = P.make(1); b->right = P.make(2);
        tests.push_back({"shape differ (L vs R)", a, b, false});
    }
    {   // same shape, one value off
        TreeNode* a = P.make(1); a->left = P.make(2); a->right = P.make(3);
        TreeNode* b = P.make(1); b->left = P.make(2); b->right = P.make(9);
        tests.push_back({"same shape, val differ", a, b, false});
    }
    {   // mirrored -> must be false
        TreeNode* a = P.make(1); a->left = P.make(2); a->right = P.make(3);
        TreeNode* b = P.make(1); b->left = P.make(3); b->right = P.make(2);
        tests.push_back({"mirrored", a, b, false});
    }
    {   // difference buried deep on the right spine
        TreeNode *a = P.make(0), *b = P.make(0), *ca = a, *cb = b;
        for (int i = 1; i < 200; i++) {
            ca->right = P.make(i); cb->right = P.make(i);
            ca = ca->right;        cb = cb->right;
        }
        ca->right = P.make(777); cb->right = P.make(778);
        tests.push_back({"deep-right mismatch", a, b, false});
    }
    {   // 10k-node left skew, identical -> recursion depth check
        TreeNode *a = P.make(0), *b = P.make(0), *ca = a, *cb = b;
        for (int i = 1; i < 10000; i++) {
            ca->left = P.make(i); cb->left = P.make(i);
            ca = ca->left;        cb = cb->left;
        }
        tests.push_back({"deep skew 10k, identical", a, b, true});
    }

    int pass = 0;
    for (auto& t : tests) {
        bool got = sol.isSameTree(t.a, t.b);   // same instance throughout: reuse-demo
        bool ok  = (got == t.exp);
        pass += ok;
        cout << (ok ? "PASS" : "FAIL") << "  " << t.name
             << " = "        << (got   ? "true" : "false")
             << "  (expected " << (t.exp ? "true" : "false") << ")\n";
    }
    cout << pass << "/" << tests.size() << " passed\n";
    return 0;
}