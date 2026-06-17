/*
 * 155. Min Stack
 * https://leetcode.com/problems/min-stack/
 *
 * Pattern: Stacks (design)
 * Time:  O(1) for push / pop / top / getMin
 * Space: O(n) - second stack mirrors the main one
 *
 * Approach:
 *   getMin in O(1) by making the minimum part of the stack's state, not a single
 *   running variable (a lone min can't be restored after you pop it).
 *   - Keep a parallel "min stack" ms alongside the value stack s.
 *   - push(v): ms stores min(v, current ms.top()) -> the min of everything at or
 *     below this level. (First element: just push v.)
 *   - pop(): pop both; the level now exposed already carries the correct min.
 *   - getMin(): read ms.top(). Nothing to recompute on pop.
 */

#include <iostream>
#include <stack>
#include <algorithm>
using namespace std;

class MinStack {
public:
    stack<int> ms;
    stack<int> s;
    MinStack() {
    }
    
    void push(int value) {
        if(!s.size())
        ms.push(value);
        else
        ms.push(min(ms.top(),value));
        s.push(value);
    }
    
    void pop() {
        s.pop();
        ms.pop();
    }
    
    int top() {
        return s.top();
    }
    
    int getMin() {
        return ms.top();
    }
};

int main() {
    MinStack st;

    // push 5, 2, 7 -> min should track the smallest so far
    st.push(5);
    st.push(2);
    st.push(7);
    cout << st.top()    << "  (expect 7)\n";   // top of value stack
    cout << st.getMin() << "  (expect 2)\n";   // min of {5,2,7}

    st.pop();                                   // remove 7
    cout << st.top()    << "  (expect 2)\n";
    cout << st.getMin() << "  (expect 2)\n";   // min restores to 2

    st.pop();                                   // remove 2  <- the min itself
    cout << st.getMin() << "  (expect 5)\n";   // min correctly restores to 5

    // duplicate-of-min case: pushing the min twice, popping once, min stays
    st.push(5);
    st.pop();
    cout << st.getMin() << "  (expect 5)\n";
    return 0;
}