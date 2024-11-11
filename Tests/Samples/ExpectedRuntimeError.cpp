#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct Node
{
    int val;
};

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    ll t;
    cin >> t;

    Node *data = nullptr;
    // This is a runtime error
    data->val = 1;

    while (t-- > 0)
    {
        int n;
        cin >> n;
        ll sum = 0;
        for (int i = 0; i < n; ++i)
        {
            ll x;
            cin >> x;
            sum += x;
        }
        cout << sum << endl;
    }
    return 0;
}