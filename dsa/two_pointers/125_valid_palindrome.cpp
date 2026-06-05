// Time: O(n) | Space: O(n)
// Approach: Build cleaned lowercase alphanumeric string, then check first vs last chars moving inward.

#include <iostream>
#include <string>
#include <cctype>
using namespace std;

class Solution {
public:
    bool isPalindrome(string s) {
        string st = "";
        for (char c : s) {
            if (!isalnum(c))
                continue;
            c = tolower(c);
            st += c;
        }
        int n = st.length();
        int i = 0;
        while (i < n / 2) {
            if (st[i] != st[n - 1 - i])
                return false;
            i++;
        }
        return true;
    }
};

int main() {
    Solution sol;

    cout << boolalpha;
    cout << sol.isPalindrome("A man, a plan, a canal: Panama") << endl;  // expect true
    cout << sol.isPalindrome("race a car") << endl;                       // expect false
    cout << sol.isPalindrome(" ") << endl;                                // expect true (empty after cleaning)
    cout << sol.isPalindrome("0P") << endl;                               // expect false (edge: digit vs letter)
    return 0;
}