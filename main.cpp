#include <iostream>
#include <vector>
#include "old_main.cpp"
using namespace std;

int op_plus(int x, int y) {
    return x + y;
}

int main() {
    vector<int> V;
    V.push_back(42);
    V.push_back(1337);
    auto x = treap<int, tsat<int, op_plus>>(V.begin(), V.end());
    cout << x.satellite().value() << endl;
    auto y = treap<int, tsat<int, op_plus>>();
    for (int i = 1; i <= 100; i++) 
        y = y.append(i);
    for (int i = 1; i <= 100; i+=2)
        y = y.set(i, -y[i]);
    cout << (x + y).slice(1, 5) << endl;
    cout << (x + y).slice(1, 5).satellite().value() << endl;
    return 0;
}
