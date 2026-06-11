// Time: O(log n)   Space: O(1)
//
// Approach:
// Standard binary search on a sorted array. Maintain a half-open interval
// [l, u) where u starts one past the end. Each step, compare the middle
// element to target and halve the search space accordingly.
 
#include <iostream>
#include <vector>
using namespace std;
 
class Solution {
public:
    int search(vector<int>& nums, int target) {
        int l = 0, u = nums.size();
        while (l < u) {
            int mid = l + (u - l) / 2;
            if (nums[mid] == target)
                return mid;
            else if (nums[mid] < target)
                l = mid + 1;
            else
                u = mid;
        }
        return -1;
    }
};
 
int main() {
    Solution s;
 
    vector<int> a = {-1, 0, 3, 5, 9, 12};
    cout << s.search(a, 9)  << "\n";  // 4
    cout << s.search(a, 2)  << "\n";  // -1
 
    vector<int> b = {5};
    cout << s.search(b, 5)  << "\n";  // 0
    cout << s.search(b, -5) << "\n";  // -1
 
    vector<int> c = {2, 5};
    cout << s.search(c, 2)  << "\n";  // 0
    cout << s.search(c, 5)  << "\n";  // 1
    cout << s.search(c, 3)  << "\n";  // -1
 
    return 0;
}
