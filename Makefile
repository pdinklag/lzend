build: lzend.cpp
	g++ --std=c++20 -O3 -g -DNDEBUG -o lzend -Irmq/include -Iordered/include libsais.c lzend.cpp
