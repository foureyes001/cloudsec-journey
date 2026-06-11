// Time: O(log(m*n))   Space: O(1)
//
// Approach:
// The matrix rows are sorted and each row starts greater than the previous
// row's last element, so the whole grid is one sorted sequence. Binary search
// over the virtual flat index range [0, m*n). Convert mid to (row, col)
// with integer division and modulo by the column count.
 
#include <iostream>
#include <vector>
using namespace std;
 
class Solution {
public:
    bool searchMatrix(vector<vector<int>>& matrix, int target) {
        int l = 0;
        int u = matrix.size() * matrix[0].size();
        int c = matrix[0].size();
 
        while (l < u) {
            int mid = l + (u - l) / 2;
            int i = mid / c;
            int j = mid % c;
 
            if (matrix[i][j] == target)
                return true;
            else if (matrix[i][j] < target)
                l = mid + 1;
            else
                u = mid;
        }
        return false;
    }
};
 
int main() {
    Solution s;
 
    vector<vector<int>> a = {{1, 3, 5, 7}, {10, 11, 16, 20}, {23, 30, 34, 60}};
    cout << s.searchMatrix(a, 3)   << "\n";  // 1 (true)
    cout << s.searchMatrix(a, 13)  << "\n";  // 0 (false)
    cout << s.searchMatrix(a, 60)  << "\n";  // 1 (last element)
 
    vector<vector<int>> b = {{1}};
    cout << s.searchMatrix(b, 1)   << "\n";  // 1
    cout << s.searchMatrix(b, 0)   << "\n";  // 0
 
    vector<vector<int>> c = {{1, 3}};
    cout << s.searchMatrix(c, 3)   << "\n";  // 1 (was the original failing case)
    cout << s.searchMatrix(c, 2)   << "\n";  // 0
 
    return 0;
}
