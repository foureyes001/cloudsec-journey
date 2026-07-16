#include <iostream>
#include <algorithm>

using namespace std;

// 1. Define the binary tree node structure (LeetCode provides this behind the scenes)
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

// 2. Your exact working solution
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode() : val(0), left(nullptr), right(nullptr) {}
 *     TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
 *     TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
 * };
 */
class Solution {
public:
    
    int height(TreeNode* root,int &diam)
    {
        if(!root)
        return -1;
        int k=1+height(root->left,diam);
        
        int n=1+height(root->right,diam);
        
        diam=max(diam,k+n);
        return max(k,n);
    }
    int diameterOfBinaryTree(TreeNode* root) {
        int diam=0;
        int n=height(root,diam);
        return diam;
    }
};

// Helper function to safely free the allocated memory and prevent memory leaks
void freeTreeMemory(TreeNode* root) {
    if (!root) return;
    freeTreeMemory(root->left);
    freeTreeMemory(root->right);
    delete root; 
}

// 3. The main function to test the code
int main() {
    // Manually constructing the tree for Example 1: [1,2,3,4,5]
    //       1
    //      / \
    //     2   3
    //    / \
    //   4   5
    
    TreeNode* root = new TreeNode(1);
    root->left = new TreeNode(2);
    root->right = new TreeNode(3);
    root->left->left = new TreeNode(4);
    root->left->right = new TreeNode(5);

    // Instantiate the class and call the function
    Solution solution;
    int result = solution.diameterOfBinaryTree(root);
    
    cout << "The diameter of the binary tree is: " << result << endl;

    // Clean up the dynamically allocated nodes before exiting
    freeTreeMemory(root);

    return 0;
}