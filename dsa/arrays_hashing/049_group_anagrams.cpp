
// Time: O(n * k log k)   Space: O(n * k)   (n = #strings, k = max string length)

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
using namespace std;

class Solution {
public:
    vector<vector<string>> groupAnagrams(vector<string>& strs) {
        unordered_map<string,vector<string>> c;
        vector<vector<string>> v;
        for(string s : strs)
        {
            string st=s;
            sort(st.begin(),st.end());
            c[st].push_back(s);
        }
        auto it=c.begin();
        while(it!=c.end())
        {
            v.push_back(it->second);
            it++;
        }
        return v;
    }
};

int main()
{
    Solution s;

    vector<string> v={"eat","tea","tan","ate","nat","bat"};
    vector<vector<string>> r=s.groupAnagrams(v);
    for(auto& g:r){ cout<<"["; for(auto& w:g) cout<<w<<" "; cout<<"] "; }
    cout<<"\n";

    v={""};
    r=s.groupAnagrams(v);
    for(auto& g:r){ cout<<"["; for(auto& w:g) cout<<w<<" "; cout<<"] "; }
    cout<<"\n";

    v={"a"};
    r=s.groupAnagrams(v);
    for(auto& g:r){ cout<<"["; for(auto& w:g) cout<<w<<" "; cout<<"] "; }
    cout<<"\n";
}