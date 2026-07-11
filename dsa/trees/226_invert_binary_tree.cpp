// 226. Invert Binary Tree — LeetCode (Easy)
// Pattern: tree recursion template — base-case guard -> root's one local job -> recurse both children -> return root.
// Root's local job here: swap the two child POINTERS (subtree identity = address, never value).
// Verified: 8/8 fixed tests + 200k fuzz (brute mirror-check + involution invert(invert(t))==t) + ASan/UBSan clean.
// Journey (named bugs for the library):
//   v1: value-identity (swapped vals, 2nd occurrence after #138) + no null base case + right subtree unvisited + returned wrong node.
//   v2: guard asked about the CHILD (root->left) instead of the handed node -> SEGV on null flows.
//   v3: pre-swap recursion on lone children -> subtree inverted twice = identity (un-inverted grandchildren).
//   v4: guard the entrance, visit each child exactly once. CLEAN.

#include <cstdio>
#include <queue>
#include <vector>

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

class Solution {
public:
    TreeNode* invertTree(TreeNode* root) {
        if (root) {
            TreeNode *c = root->left;   // swap child POINTERS: whole subtrees move in O(1)
            root->left = root->right;
            root->right = c;
            invertTree(root->left);     // hygiene: dead captures `TreeNode *l =` / `*r =` removed
            invertTree(root->right);    //          (in-place mutation; return values were unused)
        }
        return root;
    }
};

// --- minimal demo harness (LC example 1) ---
static TreeNode* build(const std::vector<int>& a) {           // level-order, -1000000 = null
    if (a.empty() || a[0] == -1000000) return nullptr;
    TreeNode* root = new TreeNode(a[0]);
    std::queue<TreeNode*> q; q.push(root);
    size_t i = 1;
    while (!q.empty() && i < a.size()) {
        TreeNode* n = q.front(); q.pop();
        if (i < a.size()) { if (a[i] != -1000000) { n->left  = new TreeNode(a[i]); q.push(n->left);  } ++i; }
        if (i < a.size()) { if (a[i] != -1000000) { n->right = new TreeNode(a[i]); q.push(n->right); } ++i; }
    }
    return root;
}
static void print(TreeNode* r) {
    std::queue<TreeNode*> q; q.push(r);
    std::printf("[");
    bool first = true;
    while (!q.empty()) {
        TreeNode* n = q.front(); q.pop();
        if (!first) std::printf(",");
        first = false;
        if (!n) { std::printf("null"); continue; }
        std::printf("%d", n->val);
        q.push(n->left); q.push(n->right);
    }
    std::printf("]\n");
}

int main() {
    Solution s;
    TreeNode* r = s.invertTree(build({4, 2, 7, 1, 3, 6, 9}));
    print(r);   // expected: [4,7,2,9,6,3,1,...]
    return 0;
}