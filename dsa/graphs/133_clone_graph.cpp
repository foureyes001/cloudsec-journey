/*
 * 133. Clone Graph  -  Medium
 *
 * Pattern:    Graph traversal with an original -> clone map.
 *             The map is both the memo and the visited marker.
 *
 * Contract:   0 <= number of nodes <= 100
 *             1 <= Node.val <= 100, val is unique per node
 *             No repeated edges, no self-loops, graph is undirected
 *             (every edge appears in both nodes' lists)
 *             Graph is connected; all nodes reachable from the given node
 *             node may be NULL (empty graph)
 *
 * Complexity: time  O((V + E) log V)  -- every node and edge visited once;
 *                                        the log V is map/set on pointer keys
 *             space O(V + E)          -- the clone graph plus the map
 *
 * Verified:   LeetCode: ACCEPTED (ASan + UBSan clean on the judge).
 *             Local harness: NOT YET RUN.  Run before trusting this line:
 *               g++ -Wall -Wextra 133_clone_graph.cpp -o out
 *               .\out.exe
 *
 * Journey:
 *   v1  new Node(...) called at EVERY encounter of a node.
 *       WRONG.  In the 4-cycle [[2,4],[1,3],[2,4],[1,3]], node 2 is reached
 *       from node 1, from node 3, and as itself -- three separate objects all
 *       claiming to be node 2, none of them wired to each other.
 *       A clone is only correct if each original maps to exactly ONE copy.
 *
 *   v2  Added a guard, but `continue` skipped the erase -> infinite loop.
 *       The guard also keyed the map on the freshly-built CLONE, a pointer
 *       that can never already be present, so it could never fire.
 *
 *   v3  Switched to map<Node*, Node*>: original -> its one clone.
 *       This is the key insight and it is the whole problem.
 *       Split into two passes (create, then wire) because node 1's neighbour
 *       list cannot be filled before node 2's clone exists, and vice versa.
 *       Still broken: the worklist was unguarded, so cycles never terminated,
 *       and the starting node was never entered into the map.
 *
 *   v4  Collapsed to ONE pass.  A clone's ADDRESS is final the moment it is
 *       created, even while its own neighbors vector is still empty -- so
 *       edges can be wired immediately and the second pass is unnecessary.
 *       Defect: the assembled vector v was never assigned to anything.
 *
 *   v5  Assigned it -- to (*s.begin())->neighbors, i.e. the ORIGINAL node.
 *       This overwrote the input graph with pointers into the clone graph.
 *       NOTE: the judge would still have printed the correct adjacency list,
 *       because the clone nodes hung off the original structure.  On this
 *       problem a wrong program can print a right answer; "it passed" is not
 *       evidence of correctness.
 *
 *   v6  Wiring target fixed to m[cur]->neighbors.
 *       ASan: heap-buffer-overflow.  Cause: (*s.begin()) was re-evaluated on
 *       every use while s.insert() ran inside the inner loop.  A newly
 *       inserted pointer can sort ahead of the current one, so `it` came from
 *       one node's neighbors vector while the loop condition tested a
 *       DIFFERENT node's end().  Switching to unordered_set does not help --
 *       a rehash relocates everything and begin() moves for a different
 *       reason.  No container makes re-querying begin() after mutation safe.
 *
 *   v7  Captured `cur = *s.begin()` once per iteration.  Overflow gone.
 *       UBSan: null dereference on adjList = [] (the empty graph).
 *
 *   v8  Guarded node == NULL.  CORRECT.
 *
 *   Grade: ASSISTED.  The assistant supplied the map's value type
 *   (original -> clone) and named the original-vs-clone wiring bug.
 *   Everything else -- the two-phase problem, the collapse to one pass, the
 *   guard design, the placement of the single guarded new -- was derived
 *   here.  The failures were bookkeeping (which variable holds which
 *   meaning), not algorithmic.
 *
 * Residual:
 *   - s.insert(*it) runs BEFORE the m.find check, so neighbours that already
 *     have clones re-enter the worklist and are popped only to hit the guard.
 *     Correct but wasteful; checking first would skip the re-insert.
 *   - The guard encodes "already wired" as "clone has neighbours".  That
 *     holds under this problem's undirected, connected constraints, but it
 *     is a weaker invariant than an explicit `done` set and would not survive
 *     a directed graph containing a zero-out-degree node reachable twice.
 *   - map/set are O(log n) on pointer keys; unordered_map/unordered_set drop
 *     the log factor.
 *
 * BUG FAMILY -- 2nd appearance, and the most serious one on record:
 *   Reading a container's bounds live while mutating a container in the same
 *   loop.  #78 bound its recursion base case to v.size() (the GROWING output)
 *   instead of nums.size().  Same shape, different container.  Unlike the
 *   signed/unsigned family this one produces genuine memory errors, so it
 *   earns a standing pre-commit check:
 *       does any loop condition re-evaluate an expression that an insert or
 *       push_back inside that loop could change?
 */

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>

using namespace std;

// Provided by LeetCode; reproduced here so the file compiles standalone.
class Node {
public:
    int val;
    vector<Node*> neighbors;
    Node() {
        val = 0;
        neighbors = vector<Node*>();
    }
    Node(int _val) {
        val = _val;
        neighbors = vector<Node*>();
    }
    Node(int _val, vector<Node*> _neighbors) {
        val = _val;
        neighbors = _neighbors;
    }
};

// ============================ SOLUTION START ============================

class Solution {
public:
    Node* cloneGraph(Node* node) {

        map<Node*, Node*> m;
        set<Node*> s = {node};
        if (!node)
            return node;
        while (!s.empty())
        {
            Node *cur = *s.begin();
            if (m.find(cur) != m.end() && m[cur]->neighbors.size())
            {
                s.erase(cur);
                continue;
            }
            else if (m.find(cur) == m.end())
            {
                Node* n = new Node(cur->val);
                m[cur] = n;
            }
            auto it = (cur)->neighbors.begin();
            vector<Node*> v;
            while (it != ((cur)->neighbors).end())
            {
                s.insert(*it);
                if (m.find(*it) == m.end())
                {
                    Node *t = new Node((*it)->val);
                    m[*it] = t;
                }
                v.push_back(m[*it]);
                it++;
            }
            m[(cur)]->neighbors = v;
            s.erase(cur);
        }
        return m[node];

    }
};

// ============================= SOLUTION END =============================

// ------------------------------ harness --------------------------------
//
// This harness checks three things, not one.  Structural equality alone is
// NOT sufficient on this problem: returning the original node, or a graph of
// clones hung off the original structure, both serialise to the correct
// adjacency list.  See Journey v5.
//
//   1. the clone serialises to the expected adjacency list
//   2. NO node in the clone is the same object as any node in the original
//   3. the original graph is byte-for-byte unchanged afterwards
//

static void collect(Node* n, set<Node*>& seen) {
    if (!n || seen.count(n)) return;
    seen.insert(n);
    for (size_t i = 0; i < n->neighbors.size(); ++i)
        collect(n->neighbors[i], seen);
}

// Node vals are 1..N, so index = val - 1.
static vector<vector<int> > serialize(Node* n) {
    set<Node*> seen;
    collect(n, seen);
    vector<pair<int, vector<int> > > rows;
    for (set<Node*>::iterator it = seen.begin(); it != seen.end(); ++it) {
        vector<int> nb;
        for (size_t i = 0; i < (*it)->neighbors.size(); ++i)
            nb.push_back((*it)->neighbors[i]->val);
        sort(nb.begin(), nb.end());
        rows.push_back(make_pair((*it)->val, nb));
    }
    sort(rows.begin(), rows.end());
    vector<vector<int> > out;
    for (size_t i = 0; i < rows.size(); ++i)
        out.push_back(rows[i].second);
    return out;
}

static Node* buildGraph(const vector<vector<int> >& adj) {
    if (adj.empty()) return NULL;
    vector<Node*> nodes(adj.size());
    for (size_t i = 0; i < adj.size(); ++i)
        nodes[i] = new Node((int)i + 1);
    for (size_t i = 0; i < adj.size(); ++i)
        for (size_t j = 0; j < adj[i].size(); ++j)
            nodes[i]->neighbors.push_back(nodes[adj[i][j] - 1]);
    return nodes[0];
}

static vector<vector<int> > normalize(vector<vector<int> > a) {
    for (size_t i = 0; i < a.size(); ++i)
        sort(a[i].begin(), a[i].end());
    return a;
}

static bool check(const char* name, vector<vector<int> > adj) {
    vector<vector<int> > expected = normalize(adj);

    Node* root = buildGraph(adj);
    vector<vector<int> > before = serialize(root);

    Solution sol;
    Node* clone = sol.cloneGraph(root);

    bool ok = true;
    string why;

    // 1. structure
    if (serialize(clone) != expected) { ok = false; why += " [structure]"; }

    // 2. deep copy - no object shared with the original
    set<Node*> origSet, cloneSet;
    collect(root, origSet);
    collect(clone, cloneSet);
    for (set<Node*>::iterator it = cloneSet.begin(); it != cloneSet.end(); ++it)
        if (origSet.count(*it)) { ok = false; why += " [SHARED NODE]"; break; }

    // 3. original untouched
    if (serialize(root) != before) { ok = false; why += " [ORIGINAL MUTATED]"; }

    // 4. exactly one clone per original
    if (cloneSet.size() != origSet.size()) { ok = false; why += " [node count]"; }

    cout << (ok ? "PASS  " : "FAIL  ") << name << why << "\n";
    return ok;
}

int main() {
    int pass = 0, total = 0;

    total++; pass += check("empty graph               []",
        vector<vector<int> >());

    total++; pass += check("single node, no edges     [[]]",
        vector<vector<int> >(1));

    {   // two nodes joined
        vector<vector<int> > a(2);
        a[0].push_back(2);
        a[1].push_back(1);
        total++; pass += check("two nodes                 [[2],[1]]", a);
    }

    {   // the 4-cycle from the problem statement
        vector<vector<int> > a(4);
        a[0].push_back(2); a[0].push_back(4);
        a[1].push_back(1); a[1].push_back(3);
        a[2].push_back(2); a[2].push_back(4);
        a[3].push_back(1); a[3].push_back(3);
        total++; pass += check("4-cycle                   [[2,4],[1,3],[2,4],[1,3]]", a);
    }

    {   // path 1-2-3-4, the two endpoints have degree 1
        vector<vector<int> > a(4);
        a[0].push_back(2);
        a[1].push_back(1); a[1].push_back(3);
        a[2].push_back(2); a[2].push_back(4);
        a[3].push_back(3);
        total++; pass += check("path                      [[2],[1,3],[2,4],[3]]", a);
    }

    {   // star: node 1 centre, three leaves
        vector<vector<int> > a(4);
        a[0].push_back(2); a[0].push_back(3); a[0].push_back(4);
        a[1].push_back(1);
        a[2].push_back(1);
        a[3].push_back(1);
        total++; pass += check("star                      [[2,3,4],[1],[1],[1]]", a);
    }

    {   // complete graph K4 - every node adjacent to every other
        vector<vector<int> > a(4);
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (i != j) a[i].push_back(j + 1);
        total++; pass += check("complete K4               [[2,3,4],[1,3,4],[1,2,4],[1,2,3]]", a);
    }

    {   // max size, cycle of 100 nodes
        const int N = 100;
        vector<vector<int> > a(N);
        for (int i = 0; i < N; ++i) {
            a[i].push_back(((i + 1) % N) + 1);
            a[i].push_back(((i - 1 + N) % N) + 1);
        }
        total++; pass += check("max size cycle            n=100", a);
    }

    {   // max size, complete-ish: node 1 connected to all others (star, n=100)
        const int N = 100;
        vector<vector<int> > a(N);
        for (int i = 1; i < N; ++i) {
            a[0].push_back(i + 1);
            a[i].push_back(1);
        }
        total++; pass += check("max size star             n=100", a);
    }

    cout << "\n" << pass << "/" << total << " passed\n";
    return pass == total ? 0 : 1;
}