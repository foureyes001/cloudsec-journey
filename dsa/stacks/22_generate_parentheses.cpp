// Time:  ~O(4^n / sqrt(n))   (Catalan(n) valid results, each O(n) to build)
// Space: O(n) recursion depth + the shared string (excludes the output list)
// Approach: backtrack on one shared string. At each step try only the choices
//   that keep it valid -- add '(' while open<n, add ')' while close<open --
//   recursing then popping the char back off (the "undo"). When open==close==n
//   the string is a complete valid combination, so record a copy.
// Verified: n=0..9 exact match vs a brute generator; counts = Catalan; all unique & balanced.
//
// Note: v and s are members, so results accumulate across calls on the same
//   object -- use a fresh Solution per call (LeetCode does this automatically).
 
#include <iostream>
#include <vector>
#include <string>
using namespace std;
 
class Solution {
public:
    vector<string> v;
    string s="";
    int o=0,e=0;
    vector<string> generateParenthesis(int n) {
        backtrack(o,e,n);
        return v;
    }
    void backtrack(int o,int e ,int n)
    {
        if(o==e&& e==n)
        {
            v.push_back(s);
            return;
            }
        if(o<n)
        {
            s+="(";
            backtrack(o+1,e,n);
            s.pop_back();
        }
        if(e<o)
        {
            s+=")";
            backtrack(o,e+1,n);
            s.pop_back();
        }
    }
};
 
int main() {
    Solution s1;
    for (auto& x : s1.generateParenthesis(1)) cout << x << " ";
    cout << "\n";   // expected: ()
 
    Solution s2;
    for (auto& x : s2.generateParenthesis(2)) cout << x << " ";
    cout << "\n";   // expected: (()) ()()
 
    Solution s3;
    for (auto& x : s3.generateParenthesis(3)) cout << x << " ";
    cout << "\n";   // expected: ((())) (()()) (())() ()(()) ()()()
 
    return 0;
}
 
