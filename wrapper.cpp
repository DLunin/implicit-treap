#include "main.cpp"
#include <boost/python.hpp>
#include <boost/python/slice.hpp>
#include <string>
#include <limits>

using namespace std;
using namespace boost::python;

typedef treap<PyObject*> Treap;

string treap___str__(const Treap *t) {
    string result;
    result += "treap([";
    bool atleast_once = false;
    for (auto current = t->begin(), end = t->end(); current != end; current++) {
        atleast_once = true;
        result += call_method<string>((*current), "__repr__");
        result += ", ";
    }
    if (atleast_once) {
        result.pop_back();
        result.pop_back();
    }
    result += "])";
    return result;
}

Treap treap_pop(const Treap *t, long long pos = -1) {
    if (pos < 0) { 
        return t->pop_back();
    }
    return t->erase(pos);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(treap_pop_overloads, treap_pop, 1, 2)

BOOST_PYTHON_MODULE(treap)
{
    class_<Treap>("Treap")
        .def("__len__", &Treap::size)
        .def("__str__", &treap___str__)
        .def("append", &Treap::push_back)
        .def("pop", &treap_pop, treap_pop_overloads(args("self", "i"), "pop"))
        .def("__getitem__", &Treap::operator[], return_value_policy<copy_const_reference>())
        .def("split", &Treap::split)
        //.def("insert", &Treap::insert)
        //.def("__getitem__", &treap___getitem__)
        .def(self + self)
        .def(self * size_t())
        .def(size_t() * self)
        //.def("set", &Treap::set)
        ;
    
    
}
