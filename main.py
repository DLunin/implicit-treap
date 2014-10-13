from treap import Treap, PersistentTreap
from sys import getrefcount

t = (Treap(PersistentTreap(Treap([[1], [2]]))))
print(getrefcount(t[0]))
