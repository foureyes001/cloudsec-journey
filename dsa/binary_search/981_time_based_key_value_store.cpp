/*
 * LC #981 — Time Based Key-Value Store
 * Pattern:   Hash map + binary search on a BOUNDARY (floor/predecessor)
 * Structure: unordered_map<key, vector<(timestamp, value)>>.
 *            set() only appends, and the problem guarantees strictly
 *            increasing timestamps -> each vector is SORTED BY CONSTRUCTION.
 *            No sorting is ever paid.
 * get():     rightmost entry with ts' <= ts (floor search — the MIRROR of
 *            #1011's "smallest feasible"). Answer-tracking form: candidate
 *            recorded on <=, then keep searching RIGHT for a later one.
 * Shape:     exclusive upper bound (l=0, u=size, while l<u, u=mid) with
 *            answer tracking — kept as verified; convention default for NEW
 *            is inclusive (annotated per #1011 precedent).
 * Edges:     missing key -> "" · all timestamps > ts -> "" (tracker never
 *            set) · exact hit valid (<=, not <).
 * Complexity: set O(1) amortized · get O(log n) per key history — optimal
 *            for a comparison-based lookup.
 * Verified:  0 fails / ~4,000,000 randomized get() calls vs linear-scan
 *            brute oracle (missing keys, below-all, exact-hit, above-all
 *            queries), ASan/UBSan clean.
 * Honest note: guided solve (2 prose-lens rounds: container levels +
 *            boundary direction; all code mine). Cold re-derive owed ~Jul 13+.
 */

#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>

class TimeMap {
public:
    std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> m;

    TimeMap() {}

    void set(std::string key, std::string value, int timestamp) {
        m[key].push_back({timestamp, value});
    }

    std::string get(std::string key, int timestamp) {
        auto it = m.find(key);
        std::string i = "";
        if (it != m.end()) {
            int l = 0, u = m[key].size();
            while (l < u) {
                int mid = (l + u) / 2;
                if (m[key][mid].first <= timestamp) {
                    i = m[key][mid].second;   // candidate; try to find a later one
                    l = mid + 1;
                } else {
                    u = mid;
                }
            }
            return i;
        }
        return "";
    }
};

// ---- driver ----
int main() {
    TimeMap tm;
    tm.set("foo", "bar", 1);
    std::printf("%s\n", tm.get("foo", 1).c_str());   // expect bar
    std::printf("%s\n", tm.get("foo", 3).c_str());   // expect bar   (floor)
    tm.set("foo", "bar2", 4);
    std::printf("%s\n", tm.get("foo", 4).c_str());   // expect bar2  (exact)
    std::printf("%s\n", tm.get("foo", 5).c_str());   // expect bar2
    std::printf("[%s]\n", tm.get("foo", 0).c_str()); // expect []    (below all)
    std::printf("[%s]\n", tm.get("nix", 9).c_str()); // expect []    (missing key)
    return 0;
}