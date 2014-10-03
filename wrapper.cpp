#include "main.cpp"
#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(treap)
{

    typedef treap<PyObject*> Treap;
    //Treap (Treap::*treap_erase1)(size_t) const = &Treap::erase;
    //Treap (Treap::*treap_erase2)(size_t, size_t) const = &Treap::erase;
    //Treap (Treap::*treap_insert1)(size_t, PyObject* const&) const = &Treap::insert;
    //Treap (Treap::*treap_insert2)(size_t, const Treap&) const = &Treap::insert;

    class_<Treap>("Treap")
        .def("__len__", &Treap::size)
        .def("append", &Treap::push_back)
        .def("__getitem__", &Treap::operator[], return_value_policy<copy_const_reference>())
        //.def("pop", &Treap::pop_back)
        //.def("set", &Treap::set)
        ;
    
    
}
