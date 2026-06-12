#include <iostream> 
#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        unordered_map<int,int> m;
        for(int i=0;i<nums.size();i++)
        {
            int need= target-nums[i];
            auto it=m.find(need);
            if(it!=m.end())
            return {i,it->second};
            m[nums[i]]=i;
            
        }
        return {};
    }
};

int main()
{
    vector<int> n;
    Solution s;
    int num ,w;
    cin>>num;
    for(int i=0;i<num;i++)
    {
        cin>>w;
        n.push_back(w);
    }
    int targ;
    cin>>targ;
    vector<int> f=s.twoSum(n,targ);
    for(int x:f)
    cout<<x<<" ";
}