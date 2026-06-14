/*
 * 206. Reverse Linked List
 * https://leetcode.com/problems/reverse-linked-list/
 * Pattern: Linked Lists (iterative pointer reversal)
 * Time:  O(n)  -- single pass
 * Space: O(1)  -- re-points existing nodes, no extra structure
 *
 * Approach: three pointers. prev starts NULL, curr starts at head. Each step:
 * stash curr->next in temp, flip curr->next to point back at prev, then advance
 * prev and curr forward. Starting prev=NULL makes the old head's next become
 * NULL for free (it's the new tail); an empty list skips the loop -> returns NULL.
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
    ListNode* reverseList(ListNode* head) {
        ListNode *curr=head;
        ListNode *prev=NULL;
        while(curr!=NULL)
        {
            ListNode *temp=curr->next;
            curr->next=prev;
            prev=curr;
            curr=temp;

        }
        return prev;
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
    printList(Solution().reverseList(buildList({1,2,3,4,5}))); // expected: 5 -> 4 -> 3 -> 2 -> 1
    printList(Solution().reverseList(buildList({1,2})));       // expected: 2 -> 1
    printList(Solution().reverseList(buildList({7})));         // expected: 7
    printList(Solution().reverseList(buildList({})));          // expected: (empty line)
    return 0;
}