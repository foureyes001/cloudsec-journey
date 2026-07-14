
int main() {
    Solution s;
    vector<int> t1 = {-1,0,1,2,-1,-4};
    auto r = s.threeSum(t1);
    for (auto& t : r) { cout << "["; for (int x : t) cout << x << " "; cout << "] "; }
    cout << "\n";   

    vector<int> t2 = {0,0,0};
    r = s.threeSum(t2);
    for (auto& t : r) { cout << "["; for (int x : t) cout << x << " "; cout << "] "; }
    cout << "\n";   

    vector<int> t3 = {-4,-2,-2,-2,0,1,2,2,2,3,3,4,4,6,6};
    r = s.threeSum(t3);
    for (auto& t : r) { cout << "["; for (int x : t) cout << x << " "; cout << "] "; }
    cout << "\n";   
}