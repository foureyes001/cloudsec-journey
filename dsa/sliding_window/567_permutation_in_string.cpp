// Pattern: Sliding Window (fixed-size)
// Time:  O(n + m)   (n=|s1|, m=|s|; per-step map compare is O(26)=O(1))
// Space: O(1)       (two freq maps, <=26 lowercase keys each)
//
// Approach: build freq map of s1; prime a window of width |s1| over s.
// Slide one char at a time (drop the left char, add the new right char) and
// compare the window's freq map to s1's. Equal map => that window is a
// permutation of s1. A fixed-width window means an exact map match is
// sufficient to confirm a permutation.
 
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;
 
class Solution {
public:
    bool checkInclusion(string s1, string s) {
        unordered_map<char,int> m,m2;
        int l=0,r=s1.length()-1,k=0;
        if(s1.length()>s.length())
        return false;
        for(char c:s1)
        {
            m[c]++;
            m2[s[k]]++;
            k++;
        }
 
 
        while(r<s.length())
        {
            if(m==m2)
            return true;
            m2[s[l]]--;
            if(!m2[s[l]])
            m2.erase(s[l]);
            r++;
            l++;
            m2[s[r]]++;
        }
        return false;
 
    }};
 
int main(){
    Solution sol;
    struct T{ string s1, s2; bool exp; };
    vector<T> tests = {
        {"ab",  "eidbaooo", true },  // "ba" present
        {"ab",  "eidboaoo", false},  // no permutation
        {"ab",  "ba",       true },  // adjacent window
        {"ab",  "ab",       true },  // n == m
        {"abc", "ab",       false},  // n > m
        {"a",   "a",        true },
        {"a",   "b",        false},
        {"ba",  "bba",      true },  // regression: v1 reset-and-skip missed this
        {"adc", "dcda",     true },
    };
 
    int pass=0;
    for(auto& t: tests){
        bool got = sol.checkInclusion(t.s1, t.s2);
        bool ok  = (got==t.exp);
        pass += ok;
        cout << (ok ? "PASS" : "FAIL")
             << "  checkInclusion(\"" << t.s1 << "\", \"" << t.s2 << "\") = "
             << (got ? "true" : "false")
             << "  (expected " << (t.exp ? "true" : "false") << ")\n";
    }
    cout << pass << "/" << tests.size() << " passed\n";
    return 0;
}
