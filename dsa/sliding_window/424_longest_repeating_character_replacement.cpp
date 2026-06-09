 /*

 * Time:    O(n)  - each index enters/leaves the window once; the inner fmax
 *                  recompute scans at most 26 map entries => O(1) constant
 * Space:   O(min(n, 26)) - one count per distinct character in the window
 *
 * Approach:
 *   Slide a window [left, right]. m[char] = count of that char inside the window.
 *   fmax = the highest single-char count currently in the window. A window is
 *   reachable with k replacements when (windowLen - fmax) <= k: keep the most
 *   frequent char, spend k turning the rest into it. The most-frequent char is
 *   chosen per-window (whatever fmax points to right now) - never a global pick.
 *   When the window is invalid, shrink from the left, decrementing counts, and
 *   recompute fmax from the (<=26-entry) map. best tracks the widest valid window.
 *
 * Note: fmax is recomputed after each shrink for clarity. There is an O(1)-per-step
 *   variant that never lowers fmax and uses `if` instead of `while` - same O(n),
 *   only a constant-factor difference - kept the clearer form here.
 */
 
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
using namespace std;
 
class Solution {
public:
    int characterReplacement(string s, int k) {
        unordered_map<char,int> m;
        int fmax=0;
        int  right=0,best=0,left=0;
        for(char c:s)
        {
            m[c]++;
            fmax=max(fmax,m[c]);
            while((right-left+1)-fmax>k)
            {
                m[s[left]]--;
                left++;
                fmax=0;
                for(auto &p:m)
                fmax=max(fmax,p.second);
 
            }
            best=max(best,right-left+1);
            right++;
        }
        return best;
 
    }
};
 
int main(){
    struct T{ string s; int k; int exp; };
    T tests[] = {
        {"ABAB",2,4}, {"AABABBA",1,4}, {"aabbbaa",2,5}, {"AAAA",2,4},
        {"ABCD",1,2}, {"ABAA",0,2}, {"AABCABBB",2,6}, {"BBBBA",1,5},
        {"ABCABC",2,4}, {"ABCDE",2,3}, {"",0,0}, {"A",5,1},
    };
    Solution sol; int passed=0, n=sizeof(tests)/sizeof(tests[0]);
    for(auto& t: tests){
        int got = sol.characterReplacement(t.s, t.k);
        bool ok = got==t.exp; passed += ok;
        cout << (ok?"PASS":"FAIL") << "  \"" << t.s << "\" k=" << t.k
             << "  exp=" << t.exp << "  got=" << got << "\n";
    }
    cout << "\n" << passed << "/" << n << " passed\n";
    return 0;
}
