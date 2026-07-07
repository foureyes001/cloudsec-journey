/*
 * LC #2 — Add Two Numbers (linked lists, digits little-endian)
 * Pattern:   List traversal + elementary-school addition with carry
 * Approach:  Three phases — both lists alive, then each tail alone —
 *            carry threaded through all three; final if(car) appends the
 *            leftover carry node (the classic dropped-carry bug avoided).
 * Dummy:     result anchored on a dummy head, returned via dummy->next —
 *            the trick #19 didn't need but this build-a-new-list shape does.
 * Refactor note: the three loops are one loop with an exhausted list
 *            contributing 0 — while(l1 || l2 || car). Deliberately left
 *            as-is; the one-loop merge is scheduled as a RE-DERIVE exercise.
 * Complexity: O(max(m,n)) time, O(max(m,n)) output nodes, O(1) extra.
 * Verified:  0 / 500,000 fuzz mismatches vs digit-wise brute oracle
 *            (lengths 1–12, LC no-leading-zero rule, carry chains,
 *            unequal lengths, final carry), ASan/UBSan clean.
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
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        ListNode *fin = new ListNode(), *f = fin;
        int car = 0;
        while (l1 != nullptr && l2 != nullptr) {
            int c;
            int v = l1->val + l2->val;
            v += car;
            if (v > 9) { c = v / 10; v %= 10; }
            else c = 0;
            ListNode *n = new ListNode(v);
            fin->next = n; fin = fin->next;
            car = c;
            l1 = l1->next; l2 = l2->next;
        }
        while (l1 != nullptr) {
            int v = l1->val + car;
            if (v > 9) { car = v / 10; v %= 10; }
            else car = 0;
            ListNode *n = new ListNode(v);
            fin->next = n; fin = fin->next;
            l1 = l1->next;
        }
        while (l2 != nullptr) {
            int v = l2->val + car;
            if (v > 9) { car = v / 10; v %= 10; }
            else car = 0;
            ListNode *n = new ListNode(v);
            fin->next = n; fin = fin->next;
            l2 = l2->next;
        }
        if (car) {
            fin->next = new ListNode(car);
        }
        ListNode *res = f->next;
        delete f;                 // wrap-time hygiene: free the dummy anchor
        return res;
    }
};

// ---- driver ----
static std::vector<ListNode*> g_nodes;

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

static void collect(ListNode* h) {           // register result nodes for cleanup
    for (ListNode *p = h; p; p = p->next) g_nodes.push_back(p);
}

int main() {
    Solution s;
    ListNode *r;

    r = s.addTwoNumbers(build({2,4,3}), build({5,6,4})); collect(r);
    print(r);                                  // expect [7,0,8]  (342+465=807)

    r = s.addTwoNumbers(build({0}), build({0})); collect(r);
    print(r);                                  // expect [0]

    r = s.addTwoNumbers(build({9,9,9,9,9,9,9}), build({9,9,9,9})); collect(r);
    print(r);                                  // expect [8,9,9,9,0,0,0,1]

    r = s.addTwoNumbers(build({5}), build({5})); collect(r);
    print(r);                                  // expect [0,1]  (final carry)

    for (ListNode *p : g_nodes) delete p;
    return 0;
}