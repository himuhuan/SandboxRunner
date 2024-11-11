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
            sum += x;
        }
        cout << sum << endl;
    }

    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            cout << "Hello, World!" << endl;
        }
    }

    return 0;
}