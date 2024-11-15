#include <bits/stdc++.h>
using namespace std;
using ll = long long;

int dfs(int n, int i) {
    
    // STACK OVERFLOW
    //if (i == n)
    //    return 1;
    
    return 1 + dfs(n, i + 1);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    cout << dfs(200, 0) << endl;

    return 0;
}