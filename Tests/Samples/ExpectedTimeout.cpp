#include <bits/stdc++.h>
using namespace std;
using ll = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    ll t;
    cin >> t;
    // Timeout!
    std::this_thread::sleep_for(std::chrono::seconds(5));
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
    return 0;
}