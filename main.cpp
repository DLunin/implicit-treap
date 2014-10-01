#include <iostream>
#include <utility>
#include <memory>
#include <queue>
#include <Python.h>
#include <type_traits>

using namespace std;

namespace nsp_treap {

template <typename T>
class node {
public:
    node(const T& val, shared_ptr<node<T>> left = nullptr, shared_ptr<node<T>> right = nullptr) : _Val(val), _Left(left), _Right(right), _Size(1 + left->size() + right->size()) { }
   
    shared_ptr<node<T>> left() { return this ? _Left : nullptr; }
    shared_ptr<node<T>> right() { return this ? _Right : nullptr; }
    const size_t size() { return this ? _Size : 0; }
    const T& val() const { return _Val; }

    const size_t height() { return this ? max(left()->height(), right()->height()) + 1 : 0; }

    void foreach(void (*f)(const node<T>* const&)) {
        if (this) {
            f(this);
            _Left->foreach(f);
            _Right->foreach(f);
        }
    }

    template <typename T1>
    friend pair<shared_ptr<node<T1>>, shared_ptr<node<T1>>> split(shared_ptr<node<T1>> tree, size_t pos);

    template <typename T1>
    friend shared_ptr<node<T1>> merge(shared_ptr<node<T1>> lhs, shared_ptr<node<T1>> rhs);

    template <typename T1>
    friend bool greater_priority(shared_ptr<node<T1>> lhs, shared_ptr<node<T1>> rhs);

    template <typename T1>
    friend bool greater_priority(shared_ptr<node<T1>> lhs); // priority compared with singleton

    template <typename T1, typename TIter> 
    friend shared_ptr<node<T1>> build(TIter begin, TIter end);

    ~node() { }
private:
    T _Val;
    size_t _Size;
    shared_ptr<node<T>> _Left, _Right;
}; 

template <typename T1>
bool greater_priority(shared_ptr<node<T1>> lhs, shared_ptr<node<T1>> rhs) {
    return (rand() % (lhs->_Size + rhs->_Size)) < lhs->_Size;
}

template <typename T1>
bool greater_priority(shared_ptr<node<T1>> lhs) {
    return (rand() % (lhs->_Size + 1)) < lhs->_Size;
}

template <typename T1>
pair<shared_ptr<node<T1>>, shared_ptr<node<T1>>> split(shared_ptr<node<T1>> tree, size_t pos) {
    if (!tree)
        return make_pair(nullptr, nullptr);
    if (tree->left()->size() >= pos) {
        auto splitted = split(tree->left(), pos);
        return make_pair(splitted.first, make_shared<node<T1>>(tree->_Val, splitted.second, tree->right()));
    }
    else {
        auto splitted = split(tree->right(), pos - tree->left()->size() - 1);
        return make_pair(make_shared<node<T1>>(tree->_Val, tree->left(), splitted.first), splitted.second);
    }
}

template <typename T1>
shared_ptr<node<T1>> merge(shared_ptr<node<T1>> lhs, shared_ptr<node<T1>> rhs) {
    if (!lhs) return rhs;
    if (!rhs) return lhs;
    if (greater_priority(lhs, rhs)) {
        return make_shared<node<T1>>(lhs->_Val, lhs->left(), merge(lhs->right(), rhs));
    }
    else {
        return make_shared<node<T1>>(rhs->_Val, merge(lhs, rhs->left()), rhs->right());
    }
}

template <typename T1, typename TIter1>
shared_ptr<node<T1>> build(TIter1 begin, TIter1 end) {
    auto path = vector<shared_ptr<node<T1>>>();
    for (TIter1 elem_ptr = begin; elem_ptr != end; elem_ptr++) {
        shared_ptr<node<T1>> prev_node_in_path = nullptr;
        while (!path.empty() && greater_priority(path.back())) {
            prev_node_in_path = path.back();
            path.pop_back();
        }
        shared_ptr<node<T1>> new_node = make_shared<node<T1>>(*elem_ptr, prev_node_in_path, nullptr);
        path.push_back(new_node);
    }
    for (size_t i = 0; i < path.size() - 1; i++) 
        path[i] = make_shared<node<T1>>(path[i]->val(), path[i]->_Left, path[i+1]);
    return path.empty() ? nullptr : path[0];
}

void debug_pyobj(PyObject *obj) {
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
}

template <typename T>
void py_node_decref(const node<T>* const& nd) {
    Py_DECREF(reinterpret_cast<PyObject*>(nd->val()));
}

template <typename T>
class treap {
public:
    treap(shared_ptr<node<T>> root = nullptr) : _Root(root) { }

    template<typename TIter>
    treap(TIter begin, TIter end) : _Root(build<T, TIter>(begin, end)) { }

    size_t size() { return _Root->size(); }

    treap<T> append(const T& x) {
        if (std::is_same<T,PyObject*>::value) {
            Py_INCREF(reinterpret_cast<PyObject*>(x));
            cout << "append: reference increased" << endl;
        }
        debug_pyobj(reinterpret_cast<PyObject*>(x));
        return treap<T>(merge(_Root, make_shared<node<T>>(x)));
    }

    const T& operator[](size_t index) const {
        auto splitted1 = split(_Root, index);
        auto splitted2 = split(splitted1.second, 1);
        const auto& result = splitted2.first->val();
        if (std::is_same<T,PyObject*>::value) {
            Py_INCREF(reinterpret_cast<PyObject*>(result));
            cout << "append: reference increased" << endl;
            debug_pyobj(reinterpret_cast<PyObject*>(result));
        }
        return result;
    }

    treap<T> set(size_t index, const T& val) {
        auto splitted1 = split(_Root, index);
        auto splitted2 = split(splitted1.second, 1);
        return merge(splitted1.first, merge(make_shared<node<T>>(val), splitted2.second));
    }

    template <typename T1>
    friend ostream& operator<<(ostream& ostr, const treap<T1>& treap);

    template <typename T1>
    friend treap<T1> operator+(const treap<T1>& lhs, const treap<T1>& rhs);

    template <typename T1>
    friend treap<T1> operator*(const treap<T1>& lhs, int n);

    size_t height() { return _Root->height(); } 

    ~treap() {
        _Root->foreach(py_node_decref);
    }
private:
    shared_ptr<node<T>> _Root;
}; 

template <typename T1>
ostream& operator<<(ostream& ostr, const treap<T1>& rhs) {
    if (rhs._Root) {
        ostr << (treap<T1>(rhs._Root->left()));
        ostr << rhs._Root->val() << ' ';
        ostr << (treap<T1>(rhs._Root->right()));
    }
    return ostr;
}

template <typename T1>
treap<T1> operator+(const treap<T1>& lhs, const treap<T1>& rhs) {
    return treap<T1>(merge(lhs._Root, rhs._Root));
}

template <typename T1>
treap<T1> operator*(const treap<T1>& lhs, int n) {
    if (n == 0)
        return treap<T1>();
    else if (n % 2)
        return (lhs * (n - 1)) + lhs;
    else {
        treap<T1> subres = lhs * (n / 2);
        return subres + subres;
    }
}
    
}

using namespace nsp_treap;

int main() {
    vector<int> V;
    V.push_back(42);
    V.push_back(1337);
    treap<int> x = treap<int>(V.begin(), V.end());
    treap<int> y = treap<int>();
    for (int i = 1; i <= 10000; i++) 
        y = y.append(i);
    for (int i = 1; i <= 10000; i+=2)
        y = y.set(i, -y[i]);
    return 0;
}
