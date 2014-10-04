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
    auto t = treap<int>();
    t = t.push_back(1);
    t = t.push_back(2);
    t = (t + t) * 10;
    for (auto current = t.begin(), end = t.end(); current != end; current++) 
        cout << *current << ' ';
    return 0;
}
