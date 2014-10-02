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
    void foreach(void (*f)(const node<T, TSat>* const)) const;

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
void node<T, TSat>::foreach(void (*f)(const node<T, TSat>* const)) const {
    if (this) {
        f(this);
        _Left->foreach(f);
        _Right->foreach(f);
    }
}

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


template <class T, class TSat=empty_sat<T>>
class treap {
public:
    treap(shared_ptr<const node<T, TSat>> root = nullptr);

    template <class TIter>
    treap(TIter begin, TIter end);

    const size_t size() const { return _Root->size(); }
    const TSat& satellite() const { return _Root->satellite(); }

    treap<T, TSat> append(const T& x) const;

    const T& operator[](size_t index) const;

    treap<T, TSat> slice(size_t begin, size_t end) const;

    treap<T, TSat> set(size_t index, const T& val) const;

    template <class T1, class TSat1>
    friend ostream& operator<<(ostream& ostr, const treap<T1, TSat1>& treap);

    template <class T1, class TSat1>
    friend treap<T1, TSat1> operator+(const treap<T1, TSat1>& lhs, const treap<T1, TSat1>& rhs);

    template <class T1, class TSat1>
    friend treap<T1, TSat1> operator*(const treap<T1, TSat1>& lhs, int n);

    const size_t height() const { return _Root->height(); } 

    ~treap() {
#ifdef PYTHON
        //_Root->foreach(py_node_decref);
#endif
    }
private:
    shared_ptr<const node<T, TSat>> _Root;
}; 


#ifdef PYTHON
void debug_pyobj(PyObject *obj) {
#ifdef DEBUG
    cout << "===== debug_pyobj =====" << endl;
    PyObject_Print(obj, stdout, Py_PRINT_RAW); cout << endl;
    cout << "id " << (long long)obj << endl;
    cout << "refcount: " << Py_REFCNT(obj) << endl;
    if (PyList_Check(obj)) {
        cout << "is list: " << PyList_Check(obj) << endl;
        cout << "size: " << PyList_Size(obj) << endl;
        cout << "elements: ";
        for (int i = 0; i < PyList_Size(obj); i++) {
            cout << PyList_GET_ITEM(obj, i) << " ";
        }
        cout << endl;
    }
    cout << "-----------------------" << endl;
#endif
}

template <class T, class TSat=empty_sat<T>>
void py_node_decref(const node<T, TSat>* const& nd) {
    Py_DECREF(reinterpret_cast<PyObject*>(nd->val()));
}

#endif

template <class T, class TSat>
treap<T, TSat>::treap(shared_ptr<const node<T, TSat>> root) : _Root(root) { }
    
template <class T, class TSat>
template <class TIter>
treap<T, TSat>::treap(TIter begin, TIter end) : _Root(build<T, TSat, TIter>(begin, end)) { }

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::append(const T& x) const {
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        Py_INCREF(reinterpret_cast<PyObject*>(x));
        debug_pyobj(reinterpret_cast<PyObject*>(x));
    }
#endif
    return treap<T, TSat>(merge(_Root, make_shared<const node<T, TSat>>(x)));
}
    
template <class T, class TSat>
const T& treap<T, TSat>::operator[](size_t index) const {
    auto splitted1 = split(_Root, index);
    auto splitted2 = split(splitted1.second, 1);
    const auto& result = splitted2.first->val();
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        Py_INCREF(reinterpret_cast<PyObject*>(result));
        debug_pyobj(reinterpret_cast<PyObject*>(result));
    }
#endif
    return result;
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::slice(size_t begin, size_t end) const {
    auto splitted1 = split(_Root, end);
    auto splitted2 = split(splitted1.first, begin);
    return splitted2.second;
}

template <class T, class TSat>
treap<T, TSat> treap<T, TSat>::set(size_t index, const T& val) const {
#ifdef PYTHON
    if (std::is_same<T,PyObject*>::value) {
        Py_INCREF(reinterpret_cast<PyObject*>(val));
        debug_pyobj(reinterpret_cast<PyObject*>(val));
    }
#endif
    auto splitted1 = split(_Root, index);
    auto splitted2 = split(splitted1.second, 1);
    return merge(splitted1.first, merge(make_shared<const node<T, TSat>>(val), splitted2.second));
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
    return treap<T1, TSat1>(merge(lhs._Root, rhs._Root));
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
