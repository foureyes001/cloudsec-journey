/*
 * 21. Merge Two Sorted Lists
 * https://leetcode.com/problems/merge-two-sorted-lists/
 * Pattern: Linked Lists (dummy/sentinel node + two-pointer splice)
 * Time:  O(n + m)  -- each node visited once
 * Space: O(1)      -- re-points existing nodes (dummy is one extra node)
 *
 * Approach: a dummy head node makes every append uniform (no "first node"
 * special case). fin (the tail) always points at the last node placed.
 * Compare c1->val and c2->val, splice the smaller node, advance that list.
 * When one list runs out, attach the remainder of the other. Return dum->next.
 */
#include <iostream>
#include <initializer_list>
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
    ListNode* mergeTwoLists(ListNode* c1, ListNode* c2) {

        ListNode *dum=new ListNode();
        ListNode *fin=dum;

        while(c1!=NULL && c2!=NULL)
        {
            if(c1->val>=c2->val)
            {
                fin->next=c2;
                fin=fin->next;
                c2=c2->next;
            }
            else
            {
                fin->next=c1;
                fin=fin->next;
                c1=c1->next;
            }
        }
        if(c1!=NULL)
        fin->next=c1;
        if(c2!=NULL)
        fin->next=c2;
        return dum->next;

    }
};

// ---- test harness (not part of the solution) ----
ListNode* buildList(initializer_list<int> vals) {
    ListNode dummy;
    ListNode* tail = &dummy;
    for (int v : vals) { tail->next = new ListNode(v); tail = tail->next; }
    return dummy.next;
}
void printList(ListNode* head) {
    while (head) { cout << head->val << (head->next ? " -> " : ""); head = head->next; }
    cout << "\n";
}

int main() {
    printList(Solution().mergeTwoLists(buildList({1,2,4}), buildList({1,3,4}))); // expected: 1 -> 1 -> 2 -> 3 -> 4 -> 4
    printList(Solution().mergeTwoLists(buildList({}),      buildList({})));      // expected: (empty line)
    printList(Solution().mergeTwoLists(buildList({}),      buildList({0})));     // expected: 0
    printList(Solution().mergeTwoLists(buildList({5}),     buildList({1,2,3}))); // expected: 1 -> 2 -> 3 -> 5
    return 0;
}