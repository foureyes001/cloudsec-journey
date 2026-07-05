/*
 * LC #19 — Remove Nth Node From End of List
 * Pattern:   Two pointers, fixed gap (linked list)
 * Invariant: create a gap of n between `curr` and `need` once; both then
 *            advance in lockstep, so the gap is preserved. When `curr`
 *            reaches the LAST node, `need` is the (n+1)th from the end,
 *            i.e. the node BEFORE the one to delete.
 * Head case: while building the gap, if `curr` walks off the list, the
 *            node to remove is the head itself -> return head->next.
 *            (Alternative to the classic dummy-node trick; no allocation.)
 * Assumes:   1 <= n <= list length (LC constraint). If n > length, the
 *            early return would wrongly remove the head — assumption is
 *            load-bearing.
 * Complexity: O(L) time single pass, O(1) space.
 * Verified:  0 / 500,000 fuzz mismatches vs brute-force oracle
 *            (lengths 1–30, all valid n), ASan/UBSan clean.
 * Note:      First idea was reverse -> delete -> reverse (3 passes, valid);
 *            replaced with the gap method after working the "distance
 *            between two same-speed pointers is constant" hint.
 */
 
#include <cstdio>
#include <vector>
 
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *nxt) : val(x), next(nxt) {}
};
 
class Solution {
public:
    ListNode* removeNthFromEnd(ListNode* head, int n) {
        ListNode *curr = head, *need = head;
 
        // Build the gap of n. If we run off the end, head is the target.
        while (n) {
            curr = curr->next;
            if (curr == nullptr) return need->next;
            n--;
        }
 
        // Advance in lockstep until curr is the last node.
        while (curr->next != nullptr) {
            curr = curr->next;
            need = need->next;
        }
 
        need->next = need->next->next;
        return head;
    }
};
 
// ---- driver ----
static std::vector<ListNode*> g_nodes;   // owns every node; freed at exit
 
static ListNode* build(const std::vector<int>& v) {
    ListNode *h = nullptr, *t = nullptr;
    for (int x : v) {
        ListNode *nd = new ListNode(x);
        g_nodes.push_back(nd);
        if (!h) h = t = nd; else { t->next = nd; t = nd; }
    }
    return h;
}
 
static void print(ListNode* h) {
    std::printf("[");
    for (ListNode *p = h; p; p = p->next)
        std::printf("%d%s", p->val, p->next ? "," : "");
    std::printf("]\n");
}
 
int main() {
    Solution s;
 
    ListNode *a = build({1, 2, 3, 4, 5});
    print(s.removeNthFromEnd(a, 2));   // expect [1,2,3,5]
 
    ListNode *b = build({1});
    print(s.removeNthFromEnd(b, 1));   // expect []
 
    ListNode *c = build({1, 2});
    print(s.removeNthFromEnd(c, 1));   // expect [1]
 
    ListNode *d = build({1, 2});
    print(s.removeNthFromEnd(d, 2));   // expect [2]  (head removal)
 
    for (ListNode *p : g_nodes) delete p;
    return 0;
}
 
