// Pattern: Binary Search (find the sorted half, then check if target is in its range)
// Time:  O(log n)
// Space: O(1)
// Approach: at each step, one half of [l, u) is fully sorted.
//   Detect it with nums[l] <= nums[mid] (left sorted) else right sorted.
//   If target lies within the SORTED half's value range, search that half; otherwise search the other.
//   Note: exclusive upper bound u = size(), so the window's right edge is nums[u-1]
//   (always in bounds: inside the loop l<u => u>=1, and u only ever shrinks so u<=size()).
 
#include <iostream>
#include <vector>
using namespace std;
 
class Solution {
public:
    int search(vector<int>& nums, int target) {
        int l=0,u=nums.size();
        while(l<u)
        {
            int mid=(l+u)/2;
            if(nums[mid]==target)
            return mid;
            if(nums[mid]>=nums[l])
            {
                if(nums[mid]>target && target>=nums[l])
                u=mid;
                else
                l=mid+1;
            }
            else
            {
                if(nums[mid]<target  && target<=nums[u-1])
                l=mid+1;
                else
                u=mid;
            }
        }
        return -1;
    }
};
 
int main(){
    Solution s;
    vector<int> a={4,5,6,7,0,1,2};
    vector<int> b={5,1,2,3,4};
    vector<int> c={1};
    cout << s.search(a, 0) << "\n"; // expected: 4
    cout << s.search(a, 2) << "\n"; // expected: 6
    cout << s.search(a, 5) << "\n"; // expected: 1
    cout << s.search(a, 3) << "\n"; // expected: -1 (absent)
    cout << s.search(b, 1) << "\n"; // expected: 1
    cout << s.search(c, 1) << "\n"; // expected: 0
    cout << s.search(c, 0) << "\n"; // expected: -1
    return 0;
}
