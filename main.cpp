#include <iostream>
#include <vector>
#include "implicit_treap.h"
#include <algorithm>
using namespace std;

int op_plus(int x, int y) {
    return x + y;
}

int op_max(int x, int y) {
    return max(x, y);
}

int op_min(int x, int y) {
    return min(x, y);
}

int main() {
    int n;
    cin >> n;
    auto v = vector<int>(n);
    auto t = treap<int>(v.begin(), v.end());
    for (int i = 0; i < 10; i++) {
        t = t + t;
    }
    cout << t.height() << endl;
    return 0;
}
