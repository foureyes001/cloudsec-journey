 /* Pattern: Linked Lists (find middle + reverse second half + merge alternately)
 * Time:  O(n)  -- three linear passes
 * Space: O(1)  -- re-points existing nodes in place (dum is one extra node)
 *
 * Approach: a composition of three sub-problems already solved this week.
 *   1) fast/slow to find the middle           (like #141)
 *   2) split at the middle, reverse 2nd half  (like #206)
 *   3) weave the two halves one node at a time, front/back/front/...  (like #21)
 * Reordered IN PLACE; head (L0) stays the first node, so the judge reads it from head.
 */
#include <iostream>
#include <vector>
using namespace std;
 
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};
 
class Solution {
public:
    void reorderList(ListNode* head) {
        ListNode *slow=head;
        ListNode *fast=head;
        while(fast!=NULL && fast->next!=NULL)
        {
            fast=fast->next->next;
            slow=slow->next;
        }
        ListNode *c1=head,*c2=slow->next,*prev=NULL;
        slow->next=NULL;
        while(c2!=NULL)
        {
            ListNode *temp=c2->next;
            c2->next=prev;
            prev=c2;
            c2=temp;
        }
        ListNode *dum=new ListNode();
        ListNode *tail=dum;
 
        while(prev!=NULL &&c1!=NULL)
        {
            tail->next=c1;
            tail=tail->next;
            c1=c1->next;
            tail->next=prev;
            tail=tail->next;
            prev=prev->next;
        }
        if(c1!=NULL)
        tail->next=c1;
    }
};
 
// ---- test harness (not part of the solution) ----
ListNode* build(vector<int> v) {
    if (v.empty()) return nullptr;
    vector<ListNode*> n;
    for (int x : v) n.push_back(new ListNode(x));
    for (size_t i = 0; i + 1 < n.size(); i++) n[i]->next = n[i+1];
    return n[0];
}
void printList(ListNode* h) {
    while (h) { cout << h->val << (h->next ? " -> " : ""); h = h->next; }
    cout << "\n";
}
 
int main() {
    ListNode* a = build({1,2,3,4,5});   Solution().reorderList(a); printList(a); // expected: 1 -> 5 -> 2 -> 4 -> 3
    ListNode* b = build({1,2,3,4});     Solution().reorderList(b); printList(b); // expected: 1 -> 4 -> 2 -> 3
    ListNode* c = build({1,2,3,4,5,6}); Solution().reorderList(c); printList(c); // expected: 1 -> 6 -> 2 -> 5 -> 3 -> 4
    ListNode* d = build({1,2,3});       Solution().reorderList(d); printList(d); // expected: 1 -> 3 -> 2
    ListNode* e = build({1,2});         Solution().reorderList(e); printList(e); // expected: 1 -> 2
    ListNode* f = build({1});           Solution().reorderList(f); printList(f); // expected: 1
    return 0;
}
