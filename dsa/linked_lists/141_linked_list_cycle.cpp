 /* Time:  O(n)  -- fast closes the gap by 1 per step; they meet within one loop traversal
 * Space: O(1)  -- two pointers, no extra structure
 *
 * Approach: slow advances 1 node, fast advances 2 nodes per iteration. Guard the
 * double-hop with (fast && fast->next). If there is a cycle, fast gains exactly 1
 * on slow each step, so the gap shrinks to 0 and they land on the same node. If the
 * list ends, fast runs off to NULL and the loop exits -> no cycle.
 */
#include <iostream>
#include <vector>
using namespace std;
 
struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(NULL) {}
};
 
class Solution {
public:
    bool hasCycle(ListNode *head) {
        ListNode *fast=head,*slow=head;
 
        while( fast!=NULL &&fast->next!=NULL)
        {
            fast=fast->next->next;
            slow=slow->next;
            if(fast==slow)
            break;
        }
        if(fast==NULL || fast->next==NULL)
        return false;
        return true;
    }
};
 
// ---- test harness (not part of the solution) ----
// pos = index the tail points back to (-1 means no cycle)
ListNode* build(vector<int> v, int pos) {
    if (v.empty()) return nullptr;
    vector<ListNode*> n;
    for (int x : v) n.push_back(new ListNode(x));
    for (size_t i = 0; i + 1 < n.size(); i++) n[i]->next = n[i+1];
    if (pos >= 0) n.back()->next = n[pos];
    return n[0];
}
 
int main() {
    cout << Solution().hasCycle(build({}, -1))        << "\n"; // expected: 0  (empty)
    cout << Solution().hasCycle(build({1}, -1))       << "\n"; // expected: 0  (single, no cycle)
    cout << Solution().hasCycle(build({1,2}, -1))     << "\n"; // expected: 0  (no cycle)
    cout << Solution().hasCycle(build({3,2,0,-4}, 1)) << "\n"; // expected: 1  (cycle -> index 1)
    cout << Solution().hasCycle(build({1,2}, 0))      << "\n"; // expected: 1  (cycle -> head)
    cout << Solution().hasCycle(build({1}, 0))        << "\n"; // expected: 1  (self-loop)
    return 0;
}
