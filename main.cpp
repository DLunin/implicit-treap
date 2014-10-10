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
        t.push_back(rand() % 100);
    int counter = 0;
    for (auto current = t.begin(); current != t.end(); current++) {
        *current = ++counter;
    }
    cout << t << endl;
    return 0;
}
