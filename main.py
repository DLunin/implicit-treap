from treap import Treap, PersistentTreap
from sys import getrefcount

t = Treap()
for i in range(10):
    t.append(i)
print(t)
