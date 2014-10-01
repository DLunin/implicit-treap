PYTHON_VERSION = 3.4
BOOST_PYTHON_LIB = boost_python-py34
PYTHON_INCLUDE = /usr/include/python$(PYTHON_VERSION)
PYTHON_LIB_DIR = /usr/lib/python$(PYTHON_VERSION)/config-3.4m-x86_64-linux-gnu
BOOST_INC = /usr/include
BOOST_LIB_DIR = /usr/lib/x86_64-linux-gnu
COMPILER = g++ -std=c++11

example.so: main.o wrapper.o
	$(COMPILER) -shared -Wl,--export-dynamic wrapper.o -L$(BOOST_LIB_DIR) -l$(BOOST_PYTHON_LIB) -L$(PYTHON_LIB_DIR) -lpython$(PYTHON_VERSION) -o treap.so

main.o: main.cpp
	$(COMPILER) -I$(PYTHON_INCLUDE) -c main.cpp -o main.o
 
wrapper.o: wrapper.cpp
	$(COMPILER) -I$(PYTHON_INCLUDE) -I$(BOOST_INC) -fPIC -c wrapper.cpp -o wrapper.o

clean:
	rm -f : *.o *.so a.out main
