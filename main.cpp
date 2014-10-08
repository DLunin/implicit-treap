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
    auto t = treap<int>();
    for (int i = 0; i < 10; i++) 
        t = t.push_back(rand() % 100);
    for (auto current = t.begin(), end = t.end(); current != end; current++) 
        cout << *current << ' ';
    return 0;
}
