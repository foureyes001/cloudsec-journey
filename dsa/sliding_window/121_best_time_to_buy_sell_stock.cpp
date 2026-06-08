/*
 * Pattern: Sliding Window (forward sweep with a running minimum)
 * Time:    O(n)  - single left-to-right pass
 * Space:   O(1)  - a handful of scalars
 *
 * Approach:
 *   Sweep forward. c = lowest price seen so far (best day to have bought).
 *   s = highest price seen since that low (best day to sell against it);
 *   when a new low appears, both reset to it. profit = s - c, and best
 *   tracks the largest profit over the whole sweep.
 *   Buy must precede sell, so the pointers only move forward - never the
 *   converge-from-ends shape used in the two-pointer problems.
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    int maxProfit(vector<int>& v) {
        int c=v[0];
        int s=v[0];
        int best=0;
        for(int k=0;k<v.size();k++)
        {
            if(c>v[k])
            {
                c=v[k];
                s=v[k];
            }
            else if(s<v[k])
            s=v[k];
            int profit=s-c;
            best=max(profit,best);
        }
        return best;
    }
};

int main(){
    struct T{ vector<int> in; int exp; };
    T tests[] = {
        {{7,1,5,3,6,4},5}, {{7,6,4,3,1},0}, {{1},0}, {{1,2,3,4,5},4},
        {{2,4,1},2}, {{3,2,6,5,0,3},4}, {{1,2},1}, {{2,1,2,1,2},1},
        {{1,5,3,6},5}, {{3,3,3},0}, {{2,1,4},3},
    };
    Solution sol; int passed=0, n=sizeof(tests)/sizeof(tests[0]);
    for(auto& t: tests){
        int got = sol.maxProfit(t.in);
        bool ok = got==t.exp; passed += ok;
        cout << (ok?"PASS":"FAIL") << "  exp=" << t.exp << "  got=" << got << "\n";
    }
    cout << "\n" << passed << "/" << n << " passed\n";
    return 0;
}