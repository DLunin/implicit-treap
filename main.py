from treap import Treap
from sys import getrefcount

x = Treap().append([1, 2, 3])
x = x.append(x)
print(x[1][0][2])
