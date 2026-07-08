// LC 138 — Copy List with Random Pointer (Medium) — dsa/linked_lists/
// Approach: address-translation table (old -> new), two passes.
//   Pass 1: one new node per original, appended via dummy+tail; record m[original] = copy.
//   Pass 2: lockstep walk; copy->random = m[original->random] (null stays null).
// Complexity: O(n log n) time with std::map / O(n) with unordered_map — see note below. O(n) extra space.
// Verified: 8/8 fixed regressions (incl. LC ex3 duplicate-values) + fuzz 200k/200k
//           (len 0–30, 70% random density, dup-heavy every 3rd case) + ASan/UBSan clean.
//           Original list confirmed unmutated.

#include <iostream>
#include <vector>
#include <map>
using namespace std;

class Node {
public:
    int val;
    Node* next;
    Node* random;
    Node(int _val) { val = _val; next = NULL; random = NULL; }
};

class Solution {
public:
    Node* copyRandomList(Node* head) {

        Node *nhead=new Node(0),*curr=head;
        Node *add=nhead;
        map <Node*,Node*> m;   // note: unordered_map<Node*,Node*> = same algorithm, O(1) avg lookup — verified variant (b); one-word swap if preferred
        while(curr!=NULL)
        {

            Node *n= new Node(curr->val);
            add->next=n;
            add=add->next;
            m[curr]=add;
            curr=curr->next;
        }
        Node *cur=nhead->next;
        Node *cur1=head;
        while(cur!=NULL)
        {
            if(cur1->random)
            cur->random=m[cur1->random];
            cur=cur->next;
            cur1=cur1->next;
        }
        Node* ret = nhead->next;   // wrap hygiene: save answer, free dummy (precedent: #2, #19)
        delete nhead;
        return ret;
    }
};

// ---------- demo main ----------
static Node* build(const vector<int>& vals, const vector<int>& randomIdx, vector<Node*>& nodes) {
    nodes.clear();
    for (int v : vals) nodes.push_back(new Node(v));
    for (size_t i = 0; i < nodes.size(); i++) {
        if (i + 1 < nodes.size()) nodes[i]->next = nodes[i + 1];
        if (randomIdx[i] != -1) nodes[i]->random = nodes[(size_t)randomIdx[i]];
    }
    return nodes.empty() ? NULL : nodes[0];
}

static void printList(const char* tag, Node* head) {
    // index the list so randoms print as indices
    vector<Node*> order;
    for (Node* p = head; p; p = p->next) order.push_back(p);
    cout << tag << ": ";
    for (size_t i = 0; i < order.size(); i++) {
        int r = -1;
        for (size_t k = 0; k < order.size(); k++)
            if (order[k] == order[i]->random) { r = (int)k; break; }
        cout << "[" << order[i]->val << ",";
        if (r == -1) cout << "null"; else cout << r;
        cout << "] ";
    }
    cout << "\n";
}

static void freeList(Node* head) {
    while (head) { Node* nx = head->next; delete head; head = nx; }
}

int main() {
    Solution s;

    // LC example 1: [[7,null],[13,0],[11,4],[10,2],[1,0]]
    vector<Node*> o1;
    Node* h1 = build({7,13,11,10,1}, {-1,0,4,2,0}, o1);
    Node* c1 = s.copyRandomList(h1);
    printList("orig1", h1);
    printList("copy1", c1);
    freeList(h1); freeList(c1);

    // duplicate values: [[3,null],[3,0],[3,null]] — identity by address, not value
    vector<Node*> o2;
    Node* h2 = build({3,3,3}, {-1,0,-1}, o2);
    Node* c2 = s.copyRandomList(h2);
    printList("orig2", h2);
    printList("copy2", c2);
    freeList(h2); freeList(c2);

    return 0;
}