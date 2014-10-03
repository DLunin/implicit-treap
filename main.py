from treap import Treap
from sys import getrefcount

x = Treap().append({1}).append([45])
x = x.append(x)
x = x.append(x)
print(x[2][1])
print(getrefcount(x[1]))
