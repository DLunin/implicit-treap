#ifndef _IMPLICIT_TREAP_H_
#define _IMPLICIT_TREAP_H_

#include <type_traits>
#include <memory>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>

#ifdef PYTHON
#include <Python.h>
#include <boost/python.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <functional>
#endif

using namespace std;

typedef int32_t treap_size_t;

namespace impl {

#ifdef PYTHON
string py_str(PyObject *obj) {
    return boost::python::call_method<string>(obj, "__str__"); 
}

string py_repr(PyObject *obj) {
    return boost::python::call_method<string>(obj, "__repr__"); 
}

string py_type(PyObject *obj) {
    string result;
    PyObject *typeobj = PyObject_Type(obj);
    result = boost::python::call_method<string>(typeobj, "__str__");
    Py_DECREF(typeobj);
}
#endif 

template <class T>
class node {
public:
    node(const T& val, 
        shared_ptr<const node<T>> left = nullptr, 
        shared_ptr<const node<T>> right = nullptr); 
   
    const shared_ptr<const node<T>> left() const;
    const shared_ptr<const node<T>> right() const;

    const treap_size_t size() const;
    const T& val() const;

#ifdef PYTHON
    const int refcount() const {
        return Py_REFCNT(reinterpret_cast<PyObject*>(val()));
    }
#endif

    template <class T1>
    friend bool greater_priority(
        shared_ptr<const node<T1>> lhs, 
        shared_ptr<const node<T1>> rhs
        );

    template <class T1>
    friend bool greater_priority(
        shared_ptr<const node<T1>> lhs
        ); // priority compared with singleton

    const treap_size_t height() const;

    template <class T1>
    friend const pair<shared_ptr<const node<T1>>, shared_ptr<const node<T1>>> split(
        shared_ptr<const node<T1>> tree, 
        treap_size_t pos
        );
    
    template <class T1>
    friend shared_ptr<const node<T1>> merge(
        shared_ptr<const node<T1>> lhs, 
        shared_ptr<const node<T1>> rhs
        );

    template <class T1, class TIter> 
    friend shared_ptr<const node<T1>> build(TIter begin, TIter end);

    ~node() {  
#ifdef PYTHON
        if (std::is_same<T,PyObject*>::value) {
            Py_DECREF(reinterpret_cast<PyObject*>(_Val));
        }
#endif
    }
private:
    T _Val;
    treap_size_t _Size;
    shared_ptr<const node<T>> _Left, _Right;
    
}; 

template <class T>
node<T>::node(const T& val, shared_ptr<const node<T>> left, shared_ptr<const node<T>> right) : _Val(val), _Left(left), _Right(right), _Size(1 + left->size() + right->size()) {
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        Py_INCREF(reinterpret_cast<PyObject*>(_Val));
    }
#endif
    // TODO: calculate operation values
}

template <class T>
const shared_ptr<const node<T>> node<T>::left() const { 
    return this ? _Left : nullptr; 
}

template <class T>
const shared_ptr<const node<T>> node<T>::right() const { 
    return this ? _Right : nullptr; 
}

template <class T>
const treap_size_t node<T>::size() const { 
    return this ? _Size : 0; 
}

template <class T>
const T& node<T>::val() const { 
    return _Val; 
}

template <class T>
const treap_size_t node<T>::height() const { 
    return this ? max(left()->height(), right()->height()) + 1 : 0; 
}

template <class T1>
bool greater_priority(shared_ptr<const node<T1>> lhs, shared_ptr<const node<T1>> rhs) {
    return (rand() % (lhs->_Size + rhs->_Size)) < lhs->_Size;
}

template <class T1>
bool greater_priority(shared_ptr<const node<T1>> lhs) {
    return (rand() % (lhs->_Size + 1)) < lhs->_Size;
}

template <class T>
void preorder_walk(shared_ptr<const node<T>> t, void (*f)(shared_ptr<const node<T>>)) {
    if (t) {
        f(t);
        preorder_walk(t->left(), f);
        preorder_walk(t->right(), f);
    }
}

template <class T>
void postorder_walk(shared_ptr<const node<T>> t, void (*f)(shared_ptr<const node<T>>)) {
    if (t) {
        postorder_walk(t->left(), f);
        postorder_walk(t->right(), f);
        f(t);
    }
}

template <class T>
void inorder_walk(shared_ptr<const node<T>> t, void (*f)(shared_ptr<const node<T>>)) {
    if (t) {
        inorder_walk(t->left(), f);
        f(t);
        inorder_walk(t->right(), f);
    }
}

#ifdef DEBUG
template <class T> 
void structural_print(shared_ptr<const node<T>> nd) {
    if (!nd) 
        return;
    if (nd->left()) {
        structural_print(nd->left());
        cerr << " "; 
    }
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        cerr << py_str(nd->val()) << ":" << nd->refcount();
    }
    else {
#endif
        cerr << nd->val();
#ifdef PYTHON
    }
#endif
    if (nd->right()) {
        cerr << " (";
        structural_print(nd->right());
        cerr << ")";
    }
}

template <class T>
void node_debug_print(shared_ptr<const node<T>> t) {
    if (t) {
        cerr << '(';
        node_debug_print(t->left());
        if (t->left())
            cerr << " ";
#ifdef PYTHON
        PyObject_Print(reinterpret_cast<PyObject*>(t->val()), stdout, Py_PRINT_RAW); 
#else
        cerr << t->val();
#endif
        if (t->right())
            cerr << " ";
        node_debug_print(t->right());
        cerr << ")";
    }
}
#ifdef PYTHON
template <class T>
void py_node_debug_print(shared_ptr<const node<T>> t) {
    if (t) {
        cerr << '(';
        py_node_debug_print(t->left());
        if (t->left())
            cerr << " ";
        cerr << Py_REFCNT(reinterpret_cast<PyObject*>(t->val()));
        if (t->right())
            cerr << " ";
        py_node_debug_print(t->right());
        cerr << ")";
    }
}
#endif
#endif

template <class T1>
const pair<shared_ptr<const node<T1>>, shared_ptr<const node<T1>>> split(
    shared_ptr<const node<T1>> tree, 
    treap_size_t pos) {
    
    if (!tree)
        return make_pair(nullptr, nullptr);
    if (tree->left()->size() >= pos) {
        auto splitted = split(tree->left(), pos);
        return make_pair(splitted.first, make_shared<const node<T1>>(tree->_Val, splitted.second, tree->right()));
    }
    else {
        auto splitted = split(tree->right(), pos - tree->left()->size() - 1);
        return make_pair(make_shared<const node<T1>>(tree->_Val, tree->left(), splitted.first), splitted.second);
    }
}

template <class T1>
shared_ptr<const node<T1>> merge(
    shared_ptr<const node<T1>> lhs, 
    shared_ptr<const node<T1>> rhs) {

    if (!lhs) return rhs;
    if (!rhs) return lhs;
    if (greater_priority(lhs, rhs)) {
        return make_shared<const node<T1>>(lhs->_Val, lhs->left(), merge(lhs->right(), rhs));
    }
    else {
        return make_shared<const node<T1>>(rhs->_Val, merge(lhs, rhs->left()), rhs->right());
    }
}

template <class T1, class TIter1>
shared_ptr<const node<T1>> build(TIter1 begin, TIter1 end) {
    auto path = vector<shared_ptr<const node<T1>>>();
    for (TIter1 elem_ptr = begin; elem_ptr != end; elem_ptr++) {
        shared_ptr<const node<T1>> prev_node_in_path = nullptr;
        while (!path.empty() && greater_priority(path.back())) {
            if (path.back()->right() != prev_node_in_path) {
                path.back() = make_shared<const node<T1>>(path.back()->val(), path.back()->left(), prev_node_in_path);
            }
            prev_node_in_path = path.back();
            path.pop_back();
        }
        shared_ptr<const node<T1>> new_node = make_shared<const node<T1>>(*elem_ptr, prev_node_in_path, nullptr);
        path.push_back(new_node);
    }
    for (treap_size_t i = path.size() - 2; i >= 0; i--) {
        if (path[i]->right() != path[i+1]) 
            path[i] = make_shared<const node<T1>>(path[i]->val(), path[i]->_Left, path[i+1]);
    }
    return path.empty() ? nullptr : path[0];
}

#ifdef PYTHON
#ifdef DEBUG
void debug_pyobj(PyObject *obj) {
    cerr << "pyobject info" << endl;
    PyObject_Print(obj, stdout, Py_PRINT_RAW); cerr << endl;
    cerr << "id " << (long long)obj << endl;
    cerr << "refcount: " << Py_REFCNT(obj) << endl;
    //if (PyList_Check(obj)) {
        //cerr << "is list: " << PyList_Check(obj) << endl;
        //cerr << "size: " << PyList_Size(obj) << endl;
        //cerr << "elements: ";
        //for (int i = 0; i < PyList_Size(obj); i++) {
            //cerr << PyList_GET_ITEM(obj, i) << " ";
        //}
        //cerr << endl;
    //}
}
#endif

void py_incref(PyObject *obj) {
    Py_INCREF(obj);
}

void py_decref(PyObject *obj) {
    Py_DECREF(obj);
}

template <class T>
void py_node_incref(shared_ptr<const node<T>> nd) {
    py_incref(reinterpret_cast<PyObject*>(nd->val()));
}

template <class T>
void py_node_decref(shared_ptr<const node<T>> nd) {
    py_decref(reinterpret_cast<PyObject*>(nd->val()));
}
#endif

template <class T>
class node_iterator : public std::iterator<forward_iterator_tag, T> {
public:
    node_iterator(shared_ptr<const node<T>> root = nullptr) {
        auto current = root;
        while (current) {
            _Path.push_back(current);
            _Right.push_back(false);
            current = current->left();
        }
        if (!_Right.empty()) 
            _Right[0] = true;
    }

    const T& operator*() const {
        const auto& result = _Path.back()->val(); 
#ifdef PYTHON
        if (std::is_same<T,PyObject*>::value) {
            py_incref(reinterpret_cast<PyObject*>(result));
        }
#endif
        return result;
    }

    const node_iterator& operator++() {
        if (_Path.back()->right()) {
            _Path.push_back(_Path.back()->right());
            _Right.push_back(true);
            while (_Path.back()->left()) {
                _Path.push_back(_Path.back()->left());
                _Right.push_back(false);
            }
        }
        else {
            _Path.pop_back();
            bool right = _Right.back();
            _Right.pop_back();
            while (!_Path.empty() && right) {
                _Path.pop_back();
                right = _Right.back();
                _Right.pop_back();
            }
        }
        return *this;
    }

    node_iterator operator++(int) {
        auto res = *this;
        operator++();
        return res;
    }

    const bool operator==(const node_iterator& rhs) const {
        if (_Path.size() != rhs._Path.size()) 
            return false;
        for (int i = 0; i < _Path.size(); i++) 
            if (_Path[i] != rhs._Path[i]) 
                return false;
        return true;
    }

    const bool operator!=(const node_iterator& rhs) const {
        return !operator==(rhs);
    }

    const bool is_end() const {
        return _Path.empty();
    }

public:
    vector<shared_ptr<const node<T>>> _Path;
    vector<bool> _Right;
};

} // namespace impl

using namespace impl;

template <class T>
class persistent_treap {
public:
    typedef node_iterator<T> const_iterator;

    persistent_treap(shared_ptr<const node<T>> root = nullptr); // v
    persistent_treap(const persistent_treap& rhs); // v
    const persistent_treap& operator=(const persistent_treap& rhs) {
#ifdef DEBUG
        debug_method("operator=");
        debug_print("lhs");
        rhs.debug_print("rhs");
#endif
        _Root = rhs._Root;
    }

    template <class TIter>
    persistent_treap(TIter begin, TIter end); 

    const treap_size_t size() const { return _Root->size(); } // v

    persistent_treap<T> push_back(const T& x) const; // v
    persistent_treap<T> push_front(const T& x) const; 
    persistent_treap<T> pop_back() const; // v
    persistent_treap<T> pop_front() const;
    
    persistent_treap<T> erase(treap_size_t pos) const;
    persistent_treap<T> erase(treap_size_t begin, treap_size_t end) const;
    persistent_treap<T> insert(treap_size_t pos, const T& val) const;
    persistent_treap<T> insert(treap_size_t pos, const persistent_treap<T>& t) const;
    pair<persistent_treap<T>, persistent_treap<T>> split(treap_size_t pos) const;

    const bool empty() const;

    const T& operator[](treap_size_t index) const;
    const T& at(treap_size_t index) const { return operator[](index); } 
    const T& back() const;
    const T& front() const;

    const bool is(const persistent_treap<T>& rhs) {
        return _Root == rhs._Root;
    }

    persistent_treap<T> slice(treap_size_t begin, treap_size_t end) const;

    persistent_treap<T> set(treap_size_t index, const T& val) const;

    template <class T1>
    friend ostream& operator<<(ostream& ostr, const persistent_treap<T1>& persistent_treap);

    template <class T1>
    friend persistent_treap<T1> operator+(const persistent_treap<T1>& lhs, const persistent_treap<T1>& rhs);

    template <class T1>
    friend persistent_treap<T1> operator*(const persistent_treap<T1>& lhs, int n);

    const bool operator==(const persistent_treap& rhs) const {
        return std::equal(cbegin(), cend(), rhs.cbegin(), rhs.cend());
    }

    const bool operator!=(const persistent_treap& rhs) const {
        return !operator==(rhs);
    }

    const bool operator<(const persistent_treap& rhs) const {
        return std::lexicographical_compare(cbegin(), cend(), rhs.cbegin(), rhs.cend());
    }

    const bool operator>(const persistent_treap& rhs) const {
        return !operator<=(rhs);
    }

    const bool operator>=(const persistent_treap& rhs) const {
        return !operator<(rhs);
    }

    const bool operator<=(const persistent_treap& rhs) const {
        return operator==(rhs) || operator<(rhs);
    }

    const treap_size_t height() const { return _Root->height(); } 

    const_iterator cbegin() const { return const_iterator(_Root); }
    const_iterator cend() const { return const_iterator(nullptr); }

    void debug_print() const {
        debug_print("this");
    }
    void debug_print(const string& name) const {
#ifdef DEBUG
        cerr << name << "[" << size() << "]: ";
        structural_print(_Root);
        cerr << endl;
#endif
    }

    static void debug_method(const string& name) {
        cerr << "method persistent_treap::" << name << " called" << endl;
    }
    
    static void debug_method_finished(const string& name) {
        cerr << "method persistent_treap::" << name << " finished" << endl;
    }
    
    ~persistent_treap() {
#ifdef DEBUG
        debug_method("DESTRUCTOR");
        debug_print();
#endif
    }
private:
    shared_ptr<const node<T>> _Root;
}; 


template <class T>
persistent_treap<T>::persistent_treap(shared_ptr<const node<T>> root) : _Root(root) { 
}

template <class T>
persistent_treap<T>::persistent_treap(const persistent_treap& rhs) : _Root(rhs._Root) {
}
    
template <class T>
template <class TIter>
persistent_treap<T>::persistent_treap(TIter begin, TIter end) : _Root(build<T, TIter>(begin, end)) { 
}

template <class T>
persistent_treap<T> persistent_treap<T>::push_back(const T& x) const {
    return (*this) + persistent_treap<T>(make_shared<node<T>>(x));
}

template <class T>
persistent_treap<T> persistent_treap<T>::push_front(const T& x) const {
    return persistent_treap<T>(make_shared<node<T>>(x)) + (*this);
}

template <class T>
persistent_treap<T> persistent_treap<T>::pop_back() const {
    return erase(size() - 1);
}

template <class T>
persistent_treap<T> persistent_treap<T>::pop_front() const {
    return erase(0);
}

template <class T>
persistent_treap<T> persistent_treap<T>::erase(treap_size_t pos) const {
    return erase(pos, pos+1);
}

template <class T>
persistent_treap<T> persistent_treap<T>::erase(treap_size_t begin, treap_size_t end) const {
    auto splitted1 = impl::split(_Root, end);
    auto splitted2 = impl::split(splitted1.first, begin);
    auto result = persistent_treap<T>(merge(splitted2.first, splitted1.second));
    return result;
}

template <class T>
persistent_treap<T> persistent_treap<T>::insert(treap_size_t pos, const T& val) const {
    return insert(pos, persistent_treap<T>(make_shared<node<T>>(val)));
}

template <class T>
persistent_treap<T> persistent_treap<T>::insert(treap_size_t pos, const persistent_treap<T>& t) const {
    auto splitted1 = impl::split(_Root, pos);
    auto result = persistent_treap<T>(merge(splitted1.first, merge(t._Root, splitted1.second)));
    return result;
}

template <class T>
pair<persistent_treap<T>, persistent_treap<T>> persistent_treap<T>::split(treap_size_t pos) const {
    auto splitted1 = impl::split(_Root, pos);
    auto result1 = persistent_treap<T>(splitted1.first);
    auto result2 = persistent_treap<T>(splitted1.second);
    return make_pair(result1, result2);
}

template <class T>
const bool persistent_treap<T>::empty() const {
    return _Root == nullptr;
}
    
template <class T>
const T& persistent_treap<T>::operator[](treap_size_t index) const {
#ifdef DEBUG
    debug_method("operator[]");
    debug_print();
#endif
    auto splitted1 = impl::split(_Root, index);
    auto splitted2 = impl::split(splitted1.second, 1);
    const auto& result = splitted2.first->val();
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        py_incref(reinterpret_cast<PyObject*>(result));
    }
#endif
#ifdef DEBUG
    debug_method_finished("operator[]");
    debug_print();
#endif
    return result;
}

template <class T>
persistent_treap<T> persistent_treap<T>::slice(treap_size_t begin, treap_size_t end) const {
    auto splitted1 = impl::split(_Root, end);
    auto splitted2 = impl::split(splitted1.first, begin);
    auto result = splitted2.second;
    return result;
}

template <class T>
persistent_treap<T> persistent_treap<T>::set(treap_size_t index, const T& val) const {
    auto splitted1 = impl::split(_Root, index);
    auto splitted2 = impl::split(splitted1.second, 1);
    auto result = persistent_treap<T>(merge(splitted1.first, merge(make_shared<const node<T>>(val), splitted2.second)));
    return result;
}

template <class T1>
ostream& operator<<(ostream& ostr, const persistent_treap<T1>& rhs) {
    if (rhs._Root) {
        ostr << (persistent_treap<T1>(rhs._Root->left()));
        ostr << rhs._Root->val() << ' ';
        ostr << (persistent_treap<T1>(rhs._Root->right()));
    }
    return ostr;
}

template <class T1>
persistent_treap<T1> operator+(const persistent_treap<T1>& lhs, const persistent_treap<T1>& rhs) {
#ifdef DEBUG
    persistent_treap<T1>::debug_method("operator[]");
    lhs.debug_print("lhs");
    rhs.debug_print("rhs");
#endif
    return persistent_treap<T1>(merge(lhs._Root, rhs._Root));
}

template <class T1>
persistent_treap<T1> operator*(const persistent_treap<T1>& lhs, int n) {
    if (n == 0)
        return persistent_treap<T1>();
    else if (n % 2)
        return (lhs * (n - 1)) + lhs;
    else {
        persistent_treap<T1> subres = lhs * (n / 2);
        return subres + subres;
    }
}

template <class T1>
persistent_treap<T1> operator*(int n, const persistent_treap<T1>& lhs) {
    return lhs * n;
}

template <class T>
class treap {
public:
    class setter {
    public:
        setter(persistent_treap<T>& t, treap_size_t pos) : _Tree(t), _Pos(pos) {  }
        operator T() { 
            return _Tree[_Pos];
        }
        const T& operator=(const T& rhs) {
            _Tree = _Tree.set(_Pos, rhs);
            return rhs;
        }
    private:
        persistent_treap<T>& _Tree;
        treap_size_t _Pos;
    };

    class iterator : public std::iterator<forward_iterator_tag, T> {
    public:
        iterator(persistent_treap<T>& t, treap_size_t pos = 0) : _Tree(t), _Pos(pos) {  }
        
        iterator operator++() {
            _Pos++;
            return *this;
        }
    
        iterator operator++(int) {
            return operator++();
        }

        setter operator*() const {
            return setter(_Tree, _Pos);
        }

        const bool operator==(const iterator& rhs) const {
            return (rhs._Tree.is(_Tree)) && _Pos == rhs._Pos;
        }   

        const bool operator!=(const iterator& rhs) const {
            return !operator==(rhs);
        }

    private:
        persistent_treap<T>& _Tree;
        treap_size_t _Pos;
    };

public:
    typedef node_iterator<T> const_iterator;

    treap(const persistent_treap<T>& tr = persistent_treap<T>()) : _Impl(tr) {
        
    }

    template <class TIter>
    treap(TIter begin, TIter end) : _Impl(begin, end) { 

    }

    const treap_size_t size() const { return _Impl.size(); } 

    void push_back(const T& x) {
        _Impl = _Impl.push_back(x);
    }
    void push_front(const T& x) {
        _Impl = _Impl.push_front(x);
    }
    void pop_back() {
        _Impl = _Impl.pop_back();
    }
    void pop_front() {
        _Impl = _Impl.pop_front();
    }
    
    void erase(treap_size_t pos) {
        _Impl = _Impl.erase(pos);
    }
    void erase(treap_size_t begin, treap_size_t end) {
        _Impl = _Impl.erase(begin, end);
    }
    void insert(treap_size_t pos, const T& val) {
        _Impl = _Impl.insert(pos, val);
    }
    void insert(treap_size_t pos, const treap<T>& t) {
        _Impl = _Impl.insert(pos, t._Impl);
    }
    
    const bool empty() const {
        return _Impl.empty();
    }

    const T& operator[](treap_size_t index) const {
        return _Impl[index];
    }
    treap<T>::setter operator[](treap_size_t index) {
        return setter(_Impl, index);
    }
    const T& at(treap_size_t index) const {
        return _Impl.at(index);
    }
    const T& back() const {
        return _Impl.back();
    }
    const T& front() const {
        return _Impl.front();
    }
    T& back() {
        return _Impl.back();
    }
    T& front() {
        return _Impl.front();
    }

    treap<T> slice(treap_size_t begin, treap_size_t end) {
        return treap<T>(_Impl.slice(begin, end));
    }

    const bool is(const treap<T>& rhs) const {
        return _Impl.is(rhs._Impl);
    }

    template <class T1>
    friend ostream& operator<<(ostream& ostr, const treap<T1>& treap);

    template <class T1>
    friend treap<T1> operator+(const treap<T1>& lhs, const treap<T1>& rhs);

    template <class T1>
    friend treap<T1> operator*(const treap<T1>& lhs, int n);

    const bool operator==(const treap& rhs) const {
        return _Impl == rhs._Impl;
    }

    const bool operator!=(const treap& rhs) const {
        return _Impl != rhs._Impl;
    }

    const bool operator<(const treap& rhs) const {
        return _Impl < rhs._Impl;
    }

    const bool operator>(const treap& rhs) const {
        return _Impl > rhs._Impl;
    }

    const bool operator>=(const treap& rhs) const {
        return _Impl >= rhs._Impl;
    }

    const bool operator<=(const treap& rhs) const {
        return _Impl <= rhs._Impl;
    }

    
    iterator begin() { return iterator(_Impl, 0); }
    iterator end() { return iterator(_Impl, size()); }
    const_iterator cbegin() const { return _Impl.cbegin(); }
    const_iterator cend() const { return _Impl.cend(); }
private:
    persistent_treap<T> _Impl;
}; 

template <class T1>
ostream& operator<<(ostream& ostr, const treap<T1>& treap) { 
    return ostr << treap._Impl;        
}

template <class T1>
treap<T1> operator+(const treap<T1>& lhs, const treap<T1>& rhs) {
    return treap<T1>(lhs._Impl + rhs._Impl);
}

template <class T1>
treap<T1> operator*(const treap<T1>& lhs, int n) {
    return treap<T1>(lhs._Impl * n);
}

template <class T1>
treap<T1> operator*(int n, const treap<T1>& rhs) {
    return rhs * n;
}

#endif

