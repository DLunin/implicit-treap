from treap import Treap, PersistentTreap
from sys import getrefcount

t = map(lambda x: x**2, Treap(range(10)))
print(Treap(t))
