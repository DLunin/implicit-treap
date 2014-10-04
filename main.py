from treap import Treap
from sys import getrefcount

x = Treap().append({1}).append([45])
x = (x + x.append('lol')) * 3
print(x)
