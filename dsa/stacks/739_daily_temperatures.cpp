/*
 
 * Time:  O(n)  - each index is pushed once and popped at most once
 * Space: O(n)  - stack holds up to n (temperature, index) pairs; answer vector is size n
 *
 * Approach:
 *   Scan left -> right keeping a stack of (temperature, index) for days still
 *   waiting for a warmer day (temperatures on the stack stay non-increasing from
 *   bottom to top). When the current day is warmer than the day on top, that day's
 *   answer is (currentIndex - itsIndex); pop it and keep going while the current
 *   day still beats the new top, then push the current day. Days left on the stack
 *   at the end never see a warmer day, so they keep their pre-filled 0.
 */

#include <vector>
#include <stack>
#include <utility>
#include <iostream>
using namespace std;

class Solution {
public:
    vector<int> dailyTemperatures(vector<int>& t) {
        stack<pair<int,int>> s;
        vector<int> v(t.size(),0);
        for(int i=0;i<t.size();i++)
        {
            if(i==0)
            {
                s.push({t[i],i});
                continue;
            }
            while(s.size() && s.top().first<t[i] )
            {
                v[s.top().second]=(i-s.top().second);
                s.pop();
            }
            s.push({t[i],i});
        }
        while(s.size())
        {
            v[s.top().second]=0;
            s.pop();
        }
        return v;
    }
};

static void print(const vector<int>& v) {
    for (size_t i = 0; i < v.size(); ++i) {
        cout << v[i];
        if (i + 1 < v.size()) cout << ' ';
    }
    cout << '\n';
}

int main() {
    Solution sol;

    vector<int> t1 = {73,74,75,71,69,72,76,73};
    vector<int> t2 = {30,40,50,60};
    vector<int> t3 = {30,60,90};
    vector<int> t4 = {50,50,50};    // edge: equal temps never count as warmer
    vector<int> t5 = {90,80,70,60}; // edge: strictly decreasing

    print(sol.dailyTemperatures(t1)); // expected: 1 1 4 2 1 1 0 0
    print(sol.dailyTemperatures(t2)); // expected: 1 1 1 0
    print(sol.dailyTemperatures(t3)); // expected: 1 1 0
    print(sol.dailyTemperatures(t4)); // expected: 0 0 0
    print(sol.dailyTemperatures(t5)); // expected: 0 0 0 0

    return 0;
}