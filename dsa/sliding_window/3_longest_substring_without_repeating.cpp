/*
 * Pattern: Sliding Window
 * Time:    O(n)              - each index enters/leaves the window once
 * Space:   O(min(n, charset)) - map holds at most one entry per distinct char
 *
 * Approach:
 *   Maintain a window [left, i] that contains only unique characters.
 *   m[char] = the most recent index that char was seen at.
 *   When the current char was seen before, jump the LEFT edge to one past
 *   that previous occurrence: left = max(left, prev + 1).
 *   The max() is the key invariant - left only ever moves FORWARD, so a stale
 *   occurrence sitting behind the window can never drag it backward.
 *   Every step the window is clean, so its width (i - left + 1) is a candidate;
 *   best tracks the widest seen.
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
using namespace std;

class Solution {
public:
    int lengthOfLongestSubstring(string s) {

        unordered_map<char,int> m;

        int best=0;
            int left=0;
        for(int i=0;i<s.length();i++)
        {
            auto it=m.find(s[i]);
            if(it!=m.end())
            {
                left=max(left,it->second+1);
            }

            best=max(best,i-left+1);
            m[s[i]]=i;
        }
        return best;
    }
};

int main(){
    struct T{ string in; int exp; };
    T tests[] = {
        {"abcabcbb",3}, {"bbbbb",1}, {"pwwkew",3}, {"",0},
        {"dvdf",3}, {"abba",2}, {"tmmzuxt",5}, {"au",2},
        {" ",1}, {"aab",2}, {"anviaj",5}, {"wobgrovw",6},
    };
    Solution sol; int passed=0, n=sizeof(tests)/sizeof(tests[0]);
    for(auto& t: tests){
        int got = sol.lengthOfLongestSubstring(t.in);
        bool ok = got==t.exp; passed += ok;
        cout << (ok?"PASS":"FAIL") << "  \"" << t.in << "\"  exp=" << t.exp << "  got=" << got << "\n";
    }
    cout << "\n" << passed << "/" << n << " passed\n";
    return 0;
}