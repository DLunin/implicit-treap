from treap import Treap
from sys import getrefcount

z = { 1 };
x = Treap().append(z)
y = Treap().append(x)
print(x[0])
print(x[0])
print(x[0])
print(x[0])
del x
print(getrefcount(z))
