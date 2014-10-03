#include <iostream>
#include <vector>
#include "implicit_treap.h"
using namespace std;

int op_plus(int x, int y) {
    return x + y;
}

int op_max(int x, int y) {
    return max(x, y);
}

int main() {
    vector<int> V;
    for (int i = 0; i < 10; i++) {
        V.push_back(rand() % 100);
    }
    auto t = treap<int, tsat<int, op_max>>(V.begin(), V.end());
    cout << t << endl;
    for (auto current = t.begin(), end = t.end(); current != end; current++) {
        cout << *current << ' ';
    }
    cout << endl;
    return 0;
}
