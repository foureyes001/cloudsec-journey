// Pattern: BINARY SEARCH ON ANSWER (min feasible capacity) — same family as 875 (Koko)
// Predicate: feasible(cap) = greedy shipsNeeded(cap) <= days  ("within" => <=, NOT ==)
//   - greedy: current load s, ship count k starts at 1; overflow => seal ship, w starts next ship
// Search space: [max(weights), sum(weights)]
//   - floor: heaviest package must fit on some ship; ceiling: days=1 carries everything
// Shape: EXCLUSIVE lower-bound (l<h, h=mid, sentinel sum+1) — matches repo's #33/#153 exclusive style;
//        convention for NEW problems is inclusive — kept as verified-written (see Day-31 handoff note).
// Complexity: O(n log(sum(weights)))   Space: O(1)
// Verified Jul 4, 2026: fuzz 500k vs brute-force oracle = 0 mismatches; ASan/UBSan clean.
// Honest note: solved over 4 iterations with lens hints (bug log in Day-31 handoff);
//              scheduled for a cold re-derive ~Jul 11+ to confirm retention.
 
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
using namespace std;
 
class Solution {
public:
    int shipWithinDays(vector<int>& weights, int days) {
        int h=accumulate(weights.begin(),weights.end(),0)+1;
        int l=*max_element(weights.begin(),weights.end()),best=h+1;
        while(l<h){
            int mid=(l+h)/2;
            int s=0,k=1;
            for(int w: weights)
            {
                if(s+w>mid)
                {
                    k++;
                    s=0;
                }
                s+=w;
            }
            if(k>days)
            l=mid+1;
            else
            {
                h=mid;
                best=min(best,mid);
            }
        }
        return best;
    }
};
 
int main() {
    Solution sol;
 
    vector<int> w1 = {1,2,3,4,5,6,7,8,9,10};
    cout << sol.shipWithinDays(w1, 5) << "\n";   // expected: 15
 
    vector<int> w2 = {3,2,2,4,1,4};
    cout << sol.shipWithinDays(w2, 3) << "\n";   // expected: 6
 
    vector<int> w3 = {1,2,3,1,1};
    cout << sol.shipWithinDays(w3, 4) << "\n";   // expected: 3 (uses 3 ships < 4 days — "within", not exact)
 
    vector<int> w4 = {5,5};
    cout << sol.shipWithinDays(w4, 1) << "\n";   // expected: 10 (answer above max element)
 
    vector<int> w5 = {8};
    cout << sol.shipWithinDays(w5, 1) << "\n";   // expected: 8 (floor of the search space)
 
    return 0;
}
