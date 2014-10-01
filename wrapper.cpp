#include "main.cpp"
#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(treap)
{
    typedef treap<PyObject*> Treap;
    class_<Treap>("Treap")
        .def("__len__", &Treap::size)
        .def("append", &Treap::append)
        .def("__getitem__", &Treap::operator[], return_value_policy<copy_const_reference>())
        .def("set", &Treap::set)
        ;
}
