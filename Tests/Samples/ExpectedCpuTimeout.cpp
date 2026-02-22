#include <bits/stdc++.h>
using namespace std;
using ll = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    ll t;
    cin >> t;

    // Drain input first so the loop below measures pure CPU behavior.
    while (t-- > 0) {
        int n;
        cin >> n;
        for (int i = 0; i < n; ++i) {
            ll x;
            cin >> x;
            (void)x;
        }
    }

    // Keep a volatile write in the loop so the compiler cannot fold it away.
    volatile ll spin = 0;
    while (true) {
        spin = spin + 1;
    }

    return 0;
}
