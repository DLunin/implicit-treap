#ifndef _IMPLICIT_TREAP_H_
#define _IMPLICIT_TREAP_H_

#include <type_traits>
#include <memory>
#include <iostream>
#include <utility>
#include <vector>

#ifdef PYTHON
#include <Python.h>
#include <boost/python.hpp>
#include <boost/function.hpp>
#include <iostream>
#endif

using namespace std;

namespace impl {

int IdleSatelliteVal = 0;
template <class T>
struct empty_sat {  
	empty_sat(const T& val) {    }
	inline void recalc_begin(const T& val) const {    }
	inline void recalc(const empty_sat<T>& val) const {    }
	inline void recalc_end(const T& val) const {    }
	const int& value() const { return IdleSatelliteVal; }
};

template <class T, T(*FOp)(T, T)>
class tsat {
public:
	tsat(const T& Val) : _Value(Val) {    }

	T& value() { return _Value; }
	const T& value() const { return _Value; }
	inline void recalc_begin(const T& Val) const {
		_Value = Val;
	}
	inline void recalc(const tsat<T, FOp>& Child) const { 
		_Value = FOp(_Value, Child.value());
	}
	inline void recalc_end(const T& Val) const {    }

	~tsat() {    }
private:
	mutable T _Value;
};

template <class T, T(*FOp)(T, T), class TNextSat>
class sat {
public:
	sat(const T& Val) : _Value(Val), _Next(Val) {    }

	const T& value() const { return _Value; }
	const TNextSat& next() const { return _Next; }
	
	inline void recalc_begin(const T& Val) const {
		_Value = Val;
		_Next.recalc_begin(Val);
	}
	inline void recalc(const sat<T, FOp, TNextSat>& Child) const { 
		_Value = FOp(_Value, Child.value());
		_Next.recalc(Child.next());
	}
	inline void recalc_end(const T& Val) const { 
		_Next.recalc_end(Val);
	}

	~sat() {    }
private:
	mutable T _Value;
	TNextSat _Next;
};

template <class T, class TSat=empty_sat<T>>
class node {
public:
    node(const T& val, 
        shared_ptr<const node<T, TSat>> left = nullptr, 
        shared_ptr<const node<T, TSat>> right = nullptr); 
   
    const shared_ptr<const node<T, TSat>> left() const;
    const shared_ptr<const node<T, TSat>> right() const;

    const size_t size() const;
    const T& val() const;
    const TSat& satellite() const;

    template <class T1, class TSat1>
    friend bool greater_priority(
        shared_ptr<const node<T1, TSat1>> lhs, 
        shared_ptr<const node<T1, TSat1>> rhs
        );

    template <class T1, class TSat1>
    friend bool greater_priority(
        shared_ptr<const node<T1, TSat1>> lhs
        ); // priority compared with singleton

    const size_t height() const;

    template <class T1, class TSat1>
    friend const pair<shared_ptr<const node<T1, TSat1>>, shared_ptr<const node<T1, TSat1>>> split(
        shared_ptr<const node<T1, TSat1>> tree, 
        size_t pos
        );
    
    template <class T1, class TSat1>
    friend shared_ptr<const node<T1, TSat1>> merge(
        shared_ptr<const node<T1, TSat1>> lhs, 
        shared_ptr<const node<T1, TSat1>> rhs
        );

    template <class T1, class TSat1, class TIter> 
    friend shared_ptr<const node<T1, TSat1>> build(TIter begin, TIter end);

    ~node() { }
private:
    T _Val;
    size_t _Size;
    shared_ptr<const node<T, TSat>> _Left, _Right;
    TSat _Satellite;
}; 

template <class T, class TSat>
node<T, TSat>::node(const T& val, shared_ptr<const node<T, TSat>> left, shared_ptr<const node<T, TSat>> right) : _Val(val), _Left(left), _Right(right), _Size(1 + left->size() + right->size()), _Satellite(_Val) {
    _Satellite.recalc_begin(_Val);
    if (_Left) 
        _Satellite.recalc(_Left->_Satellite);
    if (_Right) 
        _Satellite.recalc(_Right->_Satellite);
    _Satellite.recalc_end(_Val);
}

template <class T, class TSat>
const shared_ptr<const node<T, TSat>> node<T, TSat>::left() const { 
    return this ? _Left : nullptr; 
}

template <class T, class TSat>
const shared_ptr<const node<T, TSat>> node<T, TSat>::right() const { 
    return this ? _Right : nullptr; 
}

template <class T, class TSat>
const size_t node<T, TSat>::size() const { 
    return this ? _Size : 0; 
}

template <class T, class TSat>
const T& node<T, TSat>::val() const { 
    return _Val; 
}

template <class T, class TSat>
const TSat& node<T, TSat>::satellite() const { 
    return _Satellite;
}

template <class T, class TSat>
const size_t node<T, TSat>::height() const { 
    return this ? max(left()->height(), right()->height()) + 1 : 0; 
}

template <class T1, class TSat1>
bool greater_priority(shared_ptr<const node<T1, TSat1>> lhs, shared_ptr<const node<T1, TSat1>> rhs) {
    return (rand() % (lhs->_Size + rhs->_Size)) < lhs->_Size;
}

template <class T1, class TSat1>
bool greater_priority(shared_ptr<const node<T1, TSat1>> lhs) {
    return (rand() % (lhs->_Size + 1)) < lhs->_Size;
}

template <class T, class TSat>
void preorder_walk(shared_ptr<const node<T, TSat>> t, void (*f)(shared_ptr<const node<T, TSat>>)) {
    if (t) {
        f(t);
        preorder_walk(t->left(), f);
        preorder_walk(t->right(), f);
    }
}

template <class T, class TSat>
void postorder_walk(shared_ptr<const node<T, TSat>> t, void (*f)(shared_ptr<const node<T, TSat>>)) {
    if (t) {
        postorder_walk(t->left(), f);
        postorder_walk(t->right(), f);
        f(t);
    }
}

template <class T, class TSat>
void inorder_walk(shared_ptr<const node<T, TSat>> t, void (*f)(shared_ptr<const node<T, TSat>>)) {
    if (t) {
        inorder_walk(t->left(), f);
        f(t);
        inorder_walk(t->right(), f);
    }
}

#ifdef DEBUG
template <class T, class TSat>
void node_debug_print(shared_ptr<const node<T, TSat>> t) {
    if (t) {
        cout << '(';
        node_debug_print(t->left());
        if (t->left())
            cout << " ";
#ifdef PYTHON
        PyObject_Print(reinterpret_cast<PyObject*>(t->val()), stdout, Py_PRINT_RAW); 
#else
        cout << t->val();
#endif
        if (t->right())
            cout << " ";
        node_debug_print(t->right());
        cout << ")";
    }
}
#ifdef PYTHON
template <class T, class TSat>
void py_node_debug_print(shared_ptr<const node<T, TSat>> t) {
    if (t) {
        cout << '(';
        py_node_debug_print(t->left());
        if (t->left())
            cout << " ";
        cout << Py_REFCNT(reinterpret_cast<PyObject*>(t->val()));
        if (t->right())
            cout << " ";
        py_node_debug_print(t->right());
        cout << ")";
    }
}
#endif
#endif

template <class T1, class TSat1>
const pair<shared_ptr<const node<T1, TSat1>>, shared_ptr<const node<T1, TSat1>>> split(
    shared_ptr<const node<T1, TSat1>> tree, 
    size_t pos) {
    
    if (!tree)
        return make_pair(nullptr, nullptr);
    if (tree->left()->size() >= pos) {
        auto splitted = split(tree->left(), pos);
        return make_pair(splitted.first, make_shared<const node<T1, TSat1>>(tree->_Val, splitted.second, tree->right()));
    }
    else {
        auto splitted = split(tree->right(), pos - tree->left()->size() - 1);
        return make_pair(make_shared<const node<T1, TSat1>>(tree->_Val, tree->left(), splitted.first), splitted.second);
    }
}

template <class T1, class TSat1>
shared_ptr<const node<T1, TSat1>> merge(
    shared_ptr<const node<T1, TSat1>> lhs, 
    shared_ptr<const node<T1, TSat1>> rhs) {

    if (!lhs) return rhs;
    if (!rhs) return lhs;
    if (greater_priority(lhs, rhs)) {
        return make_shared<const node<T1, TSat1>>(lhs->_Val, lhs->left(), merge(lhs->right(), rhs));
    }
    else {
        return make_shared<const node<T1, TSat1>>(rhs->_Val, merge(lhs, rhs->left()), rhs->right());
    }
}

template <class T1, class TSat1, class TIter1>
shared_ptr<const node<T1, TSat1>> build(TIter1 begin, TIter1 end) {
    auto path = vector<shared_ptr<const node<T1, TSat1>>>();
    for (TIter1 elem_ptr = begin; elem_ptr != end; elem_ptr++) {
        shared_ptr<const node<T1, TSat1>> prev_node_in_path = nullptr;
        while (!path.empty() && greater_priority(path.back())) {
            prev_node_in_path = path.back();
            path.pop_back();
        }
        shared_ptr<const node<T1, TSat1>> new_node = make_shared<const node<T1, TSat1>>(*elem_ptr, prev_node_in_path, nullptr);
        path.push_back(new_node);
    }
    for (size_t i = 0; i < path.size() - 1; i++) 
        path[i] = make_shared<const node<T1, TSat1>>(path[i]->val(), path[i]->_Left, path[i+1]);
    return path.empty() ? nullptr : path[0];
}

#ifdef PYTHON
#ifdef DEBUG
void debug_pyobj(PyObject *obj) {
    cout << "pyobject info" << endl;
    PyObject_Print(obj, stdout, Py_PRINT_RAW); cout << endl;
    cout << "id " << (long long)obj << endl;
    cout << "refcount: " << Py_REFCNT(obj) << endl;
    //if (PyList_Check(obj)) {
        //cout << "is list: " << PyList_Check(obj) << endl;
        //cout << "size: " << PyList_Size(obj) << endl;
        //cout << "elements: ";
        //for (int i = 0; i < PyList_Size(obj); i++) {
            //cout << PyList_GET_ITEM(obj, i) << " ";
        //}
        //cout << endl;
    //}
}
#endif

void py_incref(PyObject *obj) {
    Py_INCREF(obj);
}

void py_decref(PyObject *obj) {
    Py_DECREF(obj);
}

template <class T, class TSat>
void py_node_incref(shared_ptr<const node<T, TSat>> nd) {
    py_incref(reinterpret_cast<PyObject*>(nd->val()));
}

template <class T, class TSat>
void py_node_decref(shared_ptr<const node<T, TSat>> nd) {
    py_decref(reinterpret_cast<PyObject*>(nd->val()));
}
#endif

template <class T, class TSat=empty_sat<T>>
class node_iterator {
public:
    node_iterator(shared_ptr<const node<T, TSat>> root) {
        auto current = root;
        while (current) {
            _Path.push_back(current);
            current = current->left();
        }
    }

    const T& operator*() {
        return _Path.back()->val(); 
    }

    const node_iterator& operator++() {
        if (_Path.back()->right()) {
            _Path.push_back(_Path.back()->right());
            while (_Path.back()->left()) {
                _Path.push_back(_Path.back()->left());
            }
        }
        else {
            shared_ptr<const node<T, TSat>> prev = _Path.back();
            _Path.pop_back();
            while (!_Path.empty()) {
                if (_Path.back()->left() == prev) {
                    break;
                }
                else {
                    prev = _Path.back();
                    _Path.pop_back();
                }
            }
        }
        return *this;
    }

    const node_iterator& operator++(int) {
        return operator++();
    }

    const bool operator==(const node_iterator& rhs) {
        if (_Path.size() != rhs._Path.size()) 
            return false;
        for (int i = 0; i < _Path.size(); i++) 
            if (_Path[i] != rhs._Path[i]) 
                return false;
        return true;
    }

    const bool operator!=(const node_iterator& rhs) {
        return !operator==(rhs);
    }

private:
    vector<shared_ptr<const node<T, TSat>>> _Path;
};

} // namespace impl

using namespace impl;

template <class T, class TSat=empty_sat<T>>
class treap {
public:
    typedef node_iterator<T, TSat> iterator;
    typedef node_iterator<T, TSat> const_iterator;

    treap(shared_ptr<const node<T, TSat>> root = nullptr);
    treap(const treap& rhs);

    template <class TIter>
    treap(TIter begin, TIter end);

    const size_t size() const { return _Root->size(); }
    const TSat& satellite() const { return _Root->satellite(); }

    treap<T, TSat> push_back(const T& x) const;
    treap<T, TSat> push_front(const T& x) const;
    treap<T, TSat> pop_back() const;
    treap<T, TSat> pop_front() const;
    
    treap<T, TSat> erase(size_t pos) const;
    treap<T, TSat> erase(size_t begin, size_t end) const;
    treap<T, TSat> insert(size_t pos, const T& val) const;
    treap<T, TSat> insert(size_t pos, const treap<T, TSat>& t) const;
    pair<treap<T, TSat>, treap<T, TSat>> split(size_t pos) const;

    const bool empty() const;

    const T& operator[](size_t index) const;
    const T& at(size_t index) const;
    const T& back() const;
    const T& front() const;

    treap<T, TSat> slice(size_t begin, size_t end) const;

    treap<T, TSat> set(size_t index, const T& val) const;

    template <class T1, class TSat1>
    friend ostream& operator<<(ostream& ostr, const treap<T1, TSat1>& treap);

    template <class T1, class TSat1>
    friend treap<T1, TSat1> operator+(const treap<T1, TSat1>& lhs, const treap<T1, TSat1>& rhs);

    template <class T1, class TSat1>
    friend treap<T1, TSat1> operator*(const treap<T1, TSat1>& lhs, int n);

    const size_t height() const { return _Root->height(); } 

    iterator begin() const { return iterator(_Root); }
    iterator end() const { return iterator(nullptr); }

    ~treap() {
#ifdef PYTHON
        if (std::is_same<T,PyObject*>::value) {
            postorder_walk(_Root, py_node_decref);
        }
#endif
    }
private:
    shared_ptr<const node<T, TSat>> _Root;
}; 


template <class T, class TSat>
treap<T, TSat>::treap(shared_ptr<const node<T, TSat>> root) : _Root(root) { 
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        postorder_walk(_Root, py_node_incref);
    }
#endif
}

template <class T, class TSat>
treap<T, TSat>::treap(const treap& rhs) : _Root(rhs._Root) {
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        postorder_walk(_Root, py_node_incref);
    }
#endif
}
    
template <class T, class TSat>
template <class TIter>
treap<T, TSat>::treap(TIter begin, TIter end) : _Root(build<T, TSat, TIter>(begin, end)) { 
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        postorder_walk(_Root, py_node_incref);
    }
#endif
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::push_back(const T& x) const {
    return (*this) + treap<T, TSat>(make_shared<node<T, TSat>>(x));
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::push_front(const T& x) const {
    return treap<T, TSat>(make_shared<node<T, TSat>>(x)) + (*this);
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::pop_back() const {
    return erase(size() - 1);
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::pop_front() const {
    return erase(0);
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::erase(size_t pos) const {
    return erase(pos, pos+1);
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::erase(size_t begin, size_t end) const {
    auto splitted1 = impl::split(_Root, end);
    auto splitted2 = impl::split(splitted1.first, begin);
    auto result = treap<T, TSat>(merge(splitted2.first, splitted1.second));
    return result;
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::insert(size_t pos, const T& val) const {
    return insert(pos, treap<T, TSat>(make_shared<node<T, TSat>>(val)));
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::insert(size_t pos, const treap<T, TSat>& t) const {
    auto splitted1 = impl::split(_Root, pos);
    auto result = treap<T, TSat>(merge(splitted1.first, merge(t._Root, splitted1.second)));
    return result;
}

template <class T, class TSat>
pair<treap<T, TSat>, treap<T, TSat>> treap<T, TSat>::split(size_t pos) const {
    auto splitted1 = impl::split(_Root, pos);
    auto result1 = treap<T, TSat>(splitted1.first);
    auto result2 = treap<T, TSat>(splitted1.second);
    return make_pair(result1, result2);
}

template <class T, class TSat>
const bool treap<T, TSat>::empty() const {
    return _Root == nullptr;
}
    
template <class T, class TSat>
const T& treap<T, TSat>::operator[](size_t index) const {
    auto splitted1 = impl::split(_Root, index);
    auto splitted2 = impl::split(splitted1.second, 1);
    const auto& result = splitted2.first->val();
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        py_incref(reinterpret_cast<PyObject*>(result));
    }
#endif
    return result;
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::slice(size_t begin, size_t end) const {
    auto splitted1 = impl::split(_Root, end);
    auto splitted2 = impl::split(splitted1.first, begin);
    auto result = splitted2.second;
    return result;
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::set(size_t index, const T& val) const {
    auto splitted1 = impl::split(_Root, index);
    auto splitted2 = impl::split(splitted1.second, 1);
    auto result = treap<T, TSat>(merge(splitted1.first, merge(make_shared<const node<T, TSat>>(val), splitted2.second)));
    return result;
}

template <class T1, class TSat1>
ostream& operator<<(ostream& ostr, const treap<T1, TSat1>& rhs) {
    if (rhs._Root) {
        ostr << (treap<T1, TSat1>(rhs._Root->left()));
        ostr << rhs._Root->val() << ' ';
        ostr << (treap<T1, TSat1>(rhs._Root->right()));
    }
    return ostr;
}

template <class T1, class TSat1>
treap<T1, TSat1> operator+(const treap<T1, TSat1>& lhs, const treap<T1, TSat1>& rhs) {
    auto result = treap<T1, TSat1>(merge(lhs._Root, rhs._Root));
    return result;
}

template <class T1, class TSat1>
treap<T1, TSat1> operator*(const treap<T1, TSat1>& lhs, int n) {
    if (n == 0)
        return treap<T1, TSat1>();
    else if (n % 2)
        return (lhs * (n - 1)) + lhs;
    else {
        treap<T1, TSat1> subres = lhs * (n / 2);
        return subres + subres;
    }
}

template <class T1, class TSat1>
treap<T1, TSat1> operator*(int n, const treap<T1, TSat1>& lhs) {
    return lhs * n;
}

#endif
