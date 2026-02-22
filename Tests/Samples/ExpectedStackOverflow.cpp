#include <bits/stdc++.h>
using namespace std;
using ll = long long;

#if defined(__clang__)
#define SAMPLE_NO_OPT __attribute__((optnone))
#elif defined(__GNUC__)
#define SAMPLE_NO_OPT __attribute__((optimize("O0")))
#else
#define SAMPLE_NO_OPT
#endif

SAMPLE_NO_OPT __attribute__((noinline)) void dfs(int i) {
    // Allocate a large stack frame on every recursion to trigger stack overflow quickly.
    volatile char frame[1 << 20];
    frame[0] = static_cast<char>(i & 0x7f);
    frame[(1 << 20) - 1] = frame[0];
    dfs(i + 1);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    dfs(0);

    return 0;
}
