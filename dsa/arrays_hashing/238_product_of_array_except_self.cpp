// Time: O(n)   Space: O(1) extra (output array not counted)
 
#include <iostream>
#include <vector>
using namespace std;
 
class Solution {
public:
    vector<int> productExceptSelf(vector<int>& nums) {
        vector<int> m(nums.size(),1);
        int a=1;
        for(int i=0;i<=nums.size()-1;i++)
        {
            m[i]=a;
            a*=nums[i];
 
        }
        a=1;
        for(int i=nums.size()-1;i>-1;i--)
        {
            m[i]*=a;
            a*=nums[i];
        }
        return m;
 
    }
};
 
int main()
{
    Solution s;
    vector<int> v={1,2,3,4};
    vector<int> r=s.productExceptSelf(v);
    for(int x:r) cout<<x<<" ";
    cout<<"\n";                       // expect 24 12 8 6
 
    v={-1,1,0,-3,3};
    r=s.productExceptSelf(v);
    for(int x:r) cout<<x<<" ";
    cout<<"\n";                       // expect 0 0 9 0 0
 
    v={2,2,3};
    r=s.productExceptSelf(v);
    for(int x:r) cout<<x<<" ";
    cout<<"\n";                       // expect 6 6 4
}
