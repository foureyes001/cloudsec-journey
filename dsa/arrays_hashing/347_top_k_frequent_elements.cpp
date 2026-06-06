// Time: O(n log n)   Space: O(n)
// Follow-up (better than O(n log n) via heap/bucket sort) deferred to heaps, Month 2-3.
 
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
using namespace std;
 
class Solution {
public:
    vector<int> topKFrequent(vector<int>& nums, int k) {
        unordered_map<int,int> m;
        for(int n: nums)
        {
            m[n]++;
        }
 
        vector<pair<int,int>> p;
        int i=0;
        for(auto &it:m)
        {
            p.push_back(it);
            p[i].first=it.second;
            p[i].second=it.first;
            i++;
        }
 
        sort(p.begin(),p.end(),greater<>());
        vector<int> v;
        for(auto &it:p)
        {
            v.push_back(it.second);
            k--;
            if(!k)
            return v;
        }
        return v;
 
 
    }
};
 
int main()
{
    Solution s;
    vector<int> v={1,1,1,2,2,3};
    int k=2;
    vector<int> r=s.topKFrequent(v,k);
    for(int x:r) cout<<x<<" ";
    cout<<"\n";                       // expect 1 2 (any order)
 
    v={1};
    k=1;
    r=s.topKFrequent(v,k);
    for(int x:r) cout<<x<<" ";
    cout<<"\n";                       // expect 1
 
    v={1,2,1,2,1,2,3,1,3,2};
    k=2;
    r=s.topKFrequent(v,k);
    for(int x:r) cout<<x<<" ";
    cout<<"\n";                       // expect 1 2 (any order)
}
