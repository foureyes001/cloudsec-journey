//time complexity is O(n)

#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    vector<int> twoSum(vector<int>& numbers, int target) {
        int l=0, h=numbers.size()-1;
        while(l<h)
        {
            if(numbers[l]+numbers[h]<target)
            l++;
            else if(numbers[l]+numbers[h]>target)
            h--;
            else
            return {l+1,h+1};
        }
        return {};
    }
};

int main()
{
    vector<int> v={2,7,11,15};
    int target=9;
    Solution s;
    vector<int> f=s.twoSum(v,target);
    for(int x:f)
    cout<<x<<" ";
    v={2,3,4};
    target = 6;
    f=s.twoSum(v,target);
    cout<<"\n";
    for(int x:f)
    cout<<x<<" ";
    v={-1,0};
    target = -1;
    f=s.twoSum(v,target);
    cout<<"\n";
    for(int x:f)
    cout<<x<<" ";
}