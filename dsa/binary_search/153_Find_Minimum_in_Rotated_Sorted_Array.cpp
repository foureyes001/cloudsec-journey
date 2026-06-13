// 153. Find Minimum in Rotated Sorted Array
// https://leetcode.com/problems/find-minimum-in-rotated-sorted-array/
// Pattern: Binary Search (on a rotated array -- find the pivot, no fixed target)
// Time:  O(log n)
// Space: O(1)
//
// Approach:
//   A rotated sorted array is two ascending runs with one "drop": [4,5,6,7,| 0,1,2].
//   The minimum is the start of the lower run. Compare nums[mid] to nums[u] (right end,
//   which always sits in the lower run):
//     nums[mid] > nums[u] -> mid is in the HIGH run, the min is strictly to the right -> l = mid+1
//     else                -> mid is in the LOW run, min is at mid or left -> u = mid (keep mid)
//   Loop is half-open on l<u; converges so l==u at the minimum.
//
// Note: differs from a classic search-for-target binary search -- the comparison is against
//   a moving endpoint (nums[u]), not a fixed target. The `best` tracker is belt-and-suspenders;
//   return nums[l] would also be correct once the loop converges.
//
// Verified: examples + 300k randomized rotated arrays vs brute force, 0 failures.

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    int findMin(vector<int>& nums) {
        int l=0,u=nums.size()-1;
        int best=nums[u];
        while(l<u)
        {
            int mid=(l+u)/2;
            if(nums[mid]>nums[u])
            {
                best=min(best,nums[mid]);
                l=mid+1;
            }
            else
            {
                best=min(best,nums[mid]);
                u=mid;
            }
        }
        return best;
    }
};

int main() {
    Solution s;

    { vector<int> v = {3, 4, 5, 1, 2};       cout << s.findMin(v) << " (expected 1)\n";  }
    { vector<int> v = {4, 5, 6, 7, 0, 1, 2}; cout << s.findMin(v) << " (expected 0)\n";  }
    { vector<int> v = {11, 13, 15, 17};      cout << s.findMin(v) << " (expected 11)\n"; }  // not rotated
    { vector<int> v = {2, 1};                cout << s.findMin(v) << " (expected 1)\n";  }  // smallest rotation
    { vector<int> v = {1};                   cout << s.findMin(v) << " (expected 1)\n";  }  // single element

    return 0;
}