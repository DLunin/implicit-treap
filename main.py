from treap import Treap
from sys import getrefcount

x = Treap().append({1}).append([2]).append((6,))
x = x.set(1, [100500, 1])
x = x.set(2, [100500, 1])
print(x[1])
