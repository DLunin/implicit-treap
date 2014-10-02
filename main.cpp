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
    V.push_back(1337);
    V.push_back(42);
    auto x = treap<int, sat<int, op_plus, tsat<int, op_max>>>(V.begin(), V.end());
    cout << x.satellite().next().value() << endl;
    auto y = treap<int, sat<int, op_plus, tsat<int, op_max>>>();
    for (int i = 1; i <= 100000; i++) 
        y = y.append(rand() % 1000);
    cout << y.slice(100, 99999).satellite().next().value() << endl;
    return 0;
}
