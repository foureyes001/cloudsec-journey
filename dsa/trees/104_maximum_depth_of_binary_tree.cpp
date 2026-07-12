// 104. Maximum Depth of Binary Tree — LeetCode (Easy)
// Pattern: tree recursion template — null base case -> compose children's ANSWERS -> return.
// Contract: maxDepth(node) returns the depth of the subtree rooted at node. Trust the return.
// Verified: 200k fuzz vs brute on a SINGLE reused Solution instance (stale-state killer) +
//           empty / single-node / 1000-deep right-skew edges + ASan/UBSan clean. 0 fails.
// Journey: v1 was fuzz-correct (200k clean, LC-acceptable) but used member counters (best/b) and
//          DISCARDED the recursion's return value -> stale-state fail on instance reuse
//          (call2 on single node returned 8). Named bug: missing-reset, member-variable costume
//          (cousin of #138 / #567-v1). v2 = pure function, 2-line body, state deleted. OWNED SOLO.

#include <algorithm>
#include <cstdio>

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

class Solution {
public:
    int maxDepth(TreeNode* root) {
        if (root == NULL)
            return 0;
        int k = 1 + std::max(maxDepth(root->left), maxDepth(root->right));
        return k;   // note: `return 1 + max(...)` directly is equivalent; k kept verbatim
    }
};

// --- minimal demo harness (LC example 1: [3,9,20,null,null,15,7] -> 3) ---
int main() {
    TreeNode* r = new TreeNode(3);
    r->left  = new TreeNode(9);
    r->right = new TreeNode(20);
    r->right->left  = new TreeNode(15);
    r->right->right = new TreeNode(7);
    Solution s;
    std::printf("%d\n", s.maxDepth(r));   // expected: 3
    return 0;
}