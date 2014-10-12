from treap import Treap, PersistentTreap
from sys import getrefcount

t = PersistentTreap([1, 2, 3])
t = t.set(1,5)
print(t)
