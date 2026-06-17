/*
 * 20. Valid Parentheses
 * https://leetcode.com/problems/valid-parentheses/
 *
 * Pattern: Stacks
 * Time:  O(n) - single pass over the string
 * Space: O(n) - stack holds up to n chars worst case (all openers); map is O(1)
 *
 * Approach:
 *   Brackets must close in reverse order of opening (LIFO) -> use a stack.
 *   - Map each closer to its matching opener:  ) -> (,  } -> {,  ] -> [
 *   - Opener (not a key in the map): push it.
 *   - Closer (a key in the map): the stack must be non-empty AND its top must be
 *     the matching opener; otherwise invalid. If it matches, pop.
 *   - Valid iff the stack is empty at the end (no unclosed openers left over).
 */

#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
using namespace std;

class Solution {
public:
    bool isValid(string s) {
        stack<char> p;
        unordered_map<char,char> m={{')','('},{'}','{'},{']','['}};
        for(char c:s)
        {
            
            
            auto it=m.find(c);
            if(it==m.end() )
            p.push(c);
            else
            {
                if(!p.size())
                return false;
                if(m[c]==p.top())
                {
                    p.pop();
                    continue;}
                else 
                return false;
            }
        }
        if(!p.size())
        return true;
        return false;
    }
};

int main() {
    Solution sol;

    // input    -> expected
    // ""        -> true    (empty string is valid)
    // "("       -> false   (unclosed opener)
    // ")"       -> false   (orphan closer, empty stack)
    // "()"      -> true
    // "()[]{}"  -> true
    // "(]"      -> false   (mismatched pair)
    // "([)]"    -> false   (wrong nesting order)
    // "([])"    -> true    (proper nesting)
    // "{[]}"    -> true
    // "()("     -> false   (trailing unmatched opener)

    cout << boolalpha;
    cout << sol.isValid("")       << "  (expect true)\n";
    cout << sol.isValid("(")      << "  (expect false)\n";
    cout << sol.isValid(")")      << "  (expect false)\n";
    cout << sol.isValid("()")     << "  (expect true)\n";
    cout << sol.isValid("()[]{}") << "  (expect true)\n";
    cout << sol.isValid("(]")     << "  (expect false)\n";
    cout << sol.isValid("([)]")   << "  (expect false)\n";
    cout << sol.isValid("([])")   << "  (expect true)\n";
    cout << sol.isValid("{[]}")   << "  (expect true)\n";
    cout << sol.isValid("()(")    << "  (expect false)\n";
    return 0;
}