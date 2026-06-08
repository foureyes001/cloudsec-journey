/*
 * Pattern: Two Pointers (converging from both ends)
 * Time:    O(n)  - each step moves one pointer inward; they meet once
 * Space:   O(1)
 *
 * Approach:
 *   Start a pointer at each end. The water a pair holds is
 *   min(height[i], height[j]) * (j - i), capped by the SHORTER wall.
 *   Moving the taller wall inward can never help (width shrinks, height
 *   still capped by the shorter wall), so always move the shorter side
 *   inward - that's the only move that can find a taller limiting wall.
 *   Track the largest area seen.
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    int maxArea(vector<int>& height) {
        int i=0,k=0;
        int j=height.size()-1;

        int best=0;
        while(i<j)
        {
            int area=min(height[i],height[j])*(j-i);
            best=max(area,best);
            if(height[i]<height[j])
            i++;
            else
            j--;
        }
        return best;
    }
};

int main(){
    struct T{ vector<int> in; int exp; };
    T tests[] = {
        {{1,8,6,2,5,4,8,3,7},49}, {{1,1},1}, {{4,3,2,1,4},16},
        {{1,2,1},2}, {{2,3,4,5,18,17,6},17}, {{1,2,4,3},4},
    };
    Solution sol; int passed=0, n=sizeof(tests)/sizeof(tests[0]);
    for(auto& t: tests){
        int got = sol.maxArea(t.in);
        bool ok = got==t.exp; passed += ok;
        cout << (ok?"PASS":"FAIL") << "  exp=" << t.exp << "  got=" << got << "\n";
    }
    cout << "\n" << passed << "/" << n << " passed\n";
    return 0;
}