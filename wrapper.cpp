#include "implicit_treap.h"
#include <boost/python.hpp>
#include <boost/python/slice.hpp>
#include <string>
#include <limits>

using namespace std;
using namespace boost::python;

typedef persistent_treap<PyObject*> PersistentTreap;
typedef node_iterator<PyObject*> PersistentTreapIterator;
typedef treap<PyObject*> Treap;
typedef node_iterator<PyObject*> TreapIterator;

inline object pass_through(object const& o) { return o; }

template<class TIter>
struct iterator_wrapper {
    static PyObject* next(TIter& o) {
        if (o.is_end()) {
            PyErr_SetNone(PyExc_StopIteration);
            throw_error_already_set();
            return NULL;
        }
        else {
            return *o++;
        }
    }

    static void wrap(string python_name) {
      class_<TIter>(python_name.c_str(), no_init)
        .def("__next__", next)
        .def("__iter__", pass_through)
        .def(self == self)
        .def(self != self)
      ;
    }
};

template <typename T>
string containter___str__(const T* cont) {
    string result;
    result += "[";
    bool atleast_once = false;
    for (auto current = cont->cbegin(), end = cont->cend(); current != end; current++) {
        atleast_once = true;
        result += call_method<string>((*current), "__repr__");
        result += ", ";
    }
    if (atleast_once) {
        result.pop_back();
        result.pop_back();
    }
    result += "]";
    return result;

}

string persistent_treap___str__(const PersistentTreap *t) {
    return "persistent_treap(" + containter___str__<PersistentTreap>(t) + ")";
}

string treap___str__(const Treap* t) {
    return "treap(" + containter___str__<Treap>(t) + ")";
}

PersistentTreap persistent_treap_pop(const PersistentTreap *t, long long pos = -1) {
    if (pos < 0) { 
        return t->pop_back();
    }
    return t->erase(pos);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(persistent_treap_pop_overloads, persistent_treap_pop, 1, 2)

void treap_pop(Treap *t, long long pos = -1) {
    if (pos < 0) { 
        t->pop_back();
    }
    t->erase(pos);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(treap_pop_overloads, treap_pop, 1, 2)

template <typename TContainer>
TContainer container_get_slice(TContainer *t, slice sl) {
    treap_size_t begin = 0, end = t->size();
    extract<treap_size_t> extract_begin(sl.start());  
    if (extract_begin.check()) 
        begin = extract_begin;
    extract<treap_size_t> extract_end(sl.stop());  
    if (extract_end.check())
        end = extract_end;
    return t->slice(begin, end);
}

boost::python::tuple persistent_treap_split(const PersistentTreap *t, treap_size_t pos) {
    auto res = t->split(pos);
    return boost::python::make_tuple(res.first, res.second);
}

PyObject* persistent_treap_iterator___next__(PersistentTreapIterator* self) {
    if (!self->is_end()) { 
        auto result = **self;
        (*self)++;
        return result;
    }
    else {
        PyErr_SetNone(PyExc_StopIteration);
        throw_error_already_set();
        return NULL;
    }
}

PersistentTreapIterator persistent_treap_iterator___iter__(PersistentTreapIterator* self) {
    return *self;
}

PersistentTreapIterator persistent_treap___iter__(const PersistentTreap *self) {
    return self->cbegin();
}

PyObject* treap___setitem__(Treap *self, treap_size_t pos, PyObject* value) {
    return (*self)[pos] = value;
}

class pyiter {
public:
    pyiter(PyObject *obj) {
        _Iter = PyObject_CallMethod(obj, "__iter__", NULL); 
        _Object = PyObject_CallMethod(_Iter, "__next__", NULL);
        PyErr_Clear();
    }

    pyiter(const pyiter& rhs) : _Object(rhs._Object), _Iter(rhs._Iter) {
        if (_Object) 
            Py_INCREF(_Object);
        if (_Iter) 
            Py_INCREF(_Iter);
    }

    const pyiter& operator++() {
        if (_Object) {
            Py_DECREF(_Object);
        }
        _Object = PyObject_CallMethod(_Iter, "__next__", NULL);
        PyErr_Clear();
        return *this;
    }

    const pyiter& operator++(int) {
        return operator++();
    }

    PyObject* operator*() const {
        return _Object;
    }

    const bool operator==(const pyiter& rhs) const {
        return rhs._Iter == _Iter && rhs._Object == _Object;
    }

    const bool operator!=(const pyiter& rhs) const {
        return !operator==(rhs);
    }

    const pyiter end() const {
        auto result = pyiter(*this);
        Py_DECREF(result._Object);
        result._Object = nullptr;
        return result;
    }

    ~pyiter() {
        if (_Object) 
            Py_DECREF(_Object);
        if (_Iter)
            Py_DECREF(_Iter);
    }
private:
    PyObject *_Object;
    PyObject *_Iter;
};

template <typename TCont>
shared_ptr<TCont> container___init__(PyObject *iterable) {
    auto begin = pyiter(iterable);
    auto end = begin.end();
    return make_shared<TCont>(begin, end);
}

BOOST_PYTHON_MODULE(treap)
{
    //class_<PersistentTreapIterator>("PersistentTreapIterator")
        //.def("__next__", persistent_treap_iterator___next__)
        //.def("__iter__", persistent_treap_iterator___iter__)
        //.def(self == self)
        //.def(self != self)
        //;
        
    iterator_wrapper<PersistentTreap::const_iterator>().wrap("TreapIterator");

    PersistentTreap (PersistentTreap::*persistent_treap_insert_element)(treap_size_t, PyObject* const&) const = &PersistentTreap::insert;
    PersistentTreap (PersistentTreap::*persistent_treap_insert_treap)(treap_size_t, const PersistentTreap&) const = &PersistentTreap::insert;

    PersistentTreap (PersistentTreap::*persistent_treap_erase_single)(treap_size_t) const = &PersistentTreap::erase;
    PersistentTreap (PersistentTreap::*persistent_treap_erase_range)(treap_size_t, treap_size_t) const = &PersistentTreap::erase;
    
    class_<PersistentTreap>("PersistentTreap")
        .def("__init__", make_constructor(container___init__<PersistentTreap>))
        .def("__len__", &PersistentTreap::size)
        .def("__str__", persistent_treap___str__)
        .def("__repr__", persistent_treap___str__)
        .def("__iter__", &PersistentTreap::cbegin)
        .def("append", &PersistentTreap::push_back)
        .def("pop", &persistent_treap_pop, persistent_treap_pop_overloads(args("self", "i"), "pop"))
        .def("__getitem__", &PersistentTreap::operator[], return_value_policy<copy_const_reference>())
        .def("__getitem__", container_get_slice<PersistentTreap>)
        .def("split", persistent_treap_split)
        .def("insert", persistent_treap_insert_element)
        .def("insert", persistent_treap_insert_treap)
        .def("erase", persistent_treap_erase_single)
        .def("erase", persistent_treap_erase_range)
        .def("set", &PersistentTreap::set)
        .def(self + self)
        .def(self * treap_size_t())
        .def(treap_size_t() * self)
        .def(self == self)
        .def(self != self)
        .def(self < self)
        .def(self <= self)
        .def(self > self)
        .def(self >= self)
        ;

    //iterator_wrapper<Treap::const_iterator>().wrap("TreapIterator");
    void (Treap::*treap_insert_element)(treap_size_t, PyObject* const&) = &Treap::insert;
    void (Treap::*treap_insert_treap)(treap_size_t, const Treap&) = &Treap::insert;

    void (Treap::*treap_erase_single)(treap_size_t) = &Treap::erase;
    void (Treap::*treap_erase_range)(treap_size_t, treap_size_t) = &Treap::erase;

    PyObject* const& (Treap::*treap_getitem_const)(treap_size_t) const = &Treap::operator[];

    class_<Treap>("Treap")
        .def("__init__", make_constructor(container___init__<Treap>))
        .def("__len__", &Treap::size)
        .def("__str__", treap___str__)
        .def("__repr__", treap___str__)
        .def("__iter__", &Treap::cbegin)
        .def("append", &Treap::push_back)
        .def("pop", &treap_pop, treap_pop_overloads(args("self", "i"), "pop"))
        .def("__getitem__", treap_getitem_const, return_value_policy<copy_const_reference>())
        .def("__getitem__", container_get_slice<Treap>)
        .def("__setitem__", treap___setitem__)
        .def("insert", treap_insert_element)
        .def("insert", treap_insert_treap)
        .def("erase", treap_erase_single)
        .def("erase", treap_erase_range)
        .def(self + self)
        .def(self * treap_size_t())
        .def(treap_size_t() * self)
        .def(self == self)
        .def(self != self)
        .def(self < self)
        .def(self <= self)
        .def(self > self)
        .def(self >= self)
        ;
    
}
