#include <bits/stdc++.h>
using namespace std;
using ll = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    ll t;
    cin >> t;
    while (t-- > 0) {
        int n;
        cin >> n;
        ll sum = 0;
        for (int i = 0; i < n; ++i) {
            ll x;
            cin >> x;
            // This is a CPU timeout
            for (int j = 0; j < 1000000000; ++j) {
                sum += x;
            }
        }
        cout << sum << endl;
    }
    return 0;
}