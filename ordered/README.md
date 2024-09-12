# Ordered Sets

This repository provides a straightforward C++20 header-only implementations of selected ordered set data structures.

I co-authored the SEA 2021 paper [Engineering Predecessor Data Structures for Dynamic Integer Sets](https://arxiv.org/abs/2104.06740), where previous versions of these implementations were benchmarked against different data structures, all being competitive.

### Requirements

This library is written in C++20, a corresponding compiler is required that fully supports concepts. Tests have been done only with GCC 11. Apart from that, the library as no external dependencies. For running [unit tests](#unit-tests), CMake is required.

### License

```
MIT License

Copyright (c) 2023 Patrick Dinklage

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### Unit Tests

Using CMake, you can build and run the tests using the following chain of commands in the repository root:

```sh
mkdir build; cd build
cmake ..
make
make test
```

The tests are extremely simple. The benchmark of the paper [Engineering Predecessor Data Structures for Dynamic Integer Sets](https://arxiv.org/abs/2104.06740), where the outputs of all experiments have been verified for correctness, were the actual tests.

## Data Structures

The data structures come as ordereds set as well as ordered maps (associative B-tree). They only supports unique keys; duplicate insertions of the same key lead to undefined behaviour.

### B-trees

The provided B-trees use no sophisticated tricks or magic. Let *B* be the degree of the nodes in the B-tree. The implementation supports insertion and removal operations, as well as minimum, maximum, membership, predecessor and successor queries in base-*B* logarithmic time in the number of contained keys plus node-level scans.

The implementation is very simple, almost naïve: the maintenance of the B-tree structure is implemented according to Gonazlo Navarro's book [Compact Data Structures](https://users.dcc.uchile.cl/~gnavarro/CDSbook/). The data structure for nodes is a simple array of keys which are maintained in ascending order using simple linear-time queries and manipulation. Since nodes are kept small (think a B-tree of degree 65, storing up to 64 keys at each node), thanks to caching, this is extremely fast on modern hardware and can easily outperform much more sophisticated approaches (such as fusion or burst) in the general case. Despite the simple algorithms, the code has been engineered such that it is competitive with other data structures across the board. Because B-trees are cardinality-reducing, their memory consumption depends only on the contained number of keys and not the size of the universe these keys are drawn from.

### Range Marking

The range-marking data structure (called *Dynamic Universe Sampling* in the paper) takes as a parameter the size *u* of the universe of keys (i.e., the maximum key is *u-1*), as well as a sampling parameter *s* (a power of two, set to 4096 by default). It paritions in the interval *[0, u)* of keys into buckets of size *s*. Operations and queries on a key are delegated to the corresponding bucket.

Only active (non-empty) buckets are actually allocated. If queries must find a bucket (e.g., a succeeding bucket for successor search), this is done using linear scans and thus takes time *O(u/s)*.

The buckets are maintained as bit vectors of length *s*. This means that inserting or removing keys is done in constant time by simply setting or clearing a bit, respectively. Queries are implemented as linear scans over the (word-packed) bits, taking time *O(s/w+w)* if *w* is the word size (e.g., 64 bits).

The algorithms behind this data structure are very straightforward. The memory consumption directly depends on the universe size *u*. Keys must be unsigned integrals. Attempts to insert, remove or query keys greater than *u-1* is undefined.

### Which to use?

The range-marking data structure is typically much faster than B-trees thanks to their simple structure and few indirections. If you can model your keys as unsigned integrals and their range is reasonably restricted, it should be the data structure of choice in most cases.

For all other cases, B-trees are a suitable solution for the general case.

## Usage

The library is header only, so all you need to do is make sure it's in your include path.

In case you use CMake, you can embed this repository into yours (e.g., as a git submodule) and add it like so:

```cmake
add_subdirectory(path/to/ordered)
```

You can then link against the `ordered` interface library, which will automatically add the include directory to your target.

### QueryResult

The queries *find*, *min*, *max*, *predecessor* and *successor* return an instance of the `ordered:QueryResult<Key, Value>` struct.

First, it contains the flag `exists` telling whether a result was found for the query key at all. Casting query results to `bool` will also return the `exists` flag. Second, it contains the `key` field that will hold the corresponding key, e.g., the predecessor of the query key. Last, the `value` field will hold the value associated to `key`, if any. In case you are using sets as opposed to maps, the `value` will be a meaningless dummy that you may ignore.

If `exists` is `false`, the contents of `key` and `value` are undefined.

### Ordered Sets

The aliases `ordered::btree::Set<Key>` and `ordered::range_marking::Set<Key>` give you the classic ordered set experience for keys of type `Key`. For range-marking, `Key` must be an unsigned integral type, for B-trees it must satisfy `std::totally_ordered`.

Note that even though no values are associated with keys here, the [QueryResult](#QueryResult) from queries will have a `value` field. This is simply for convenience of keeping the code succinct; it is not meaningful and can simply be ignored in these cases.

##### Example

The following excerpt from the tests should tell you all you need to know about the usage.

```cpp
#include <ordered/btree.hpp>
#include <ordered/marked.hpp>

// start with an empty set
ordered::btree::Set<int> set;
// alternatively, using maximum possible key 15
// ordered::range_marking::Set<unsigned int> set(15);

CHECK(set.empty());

// insert some numbers
set.insert(5);
set.insert(1);
set.insert(8);
set.insert(4);
set.insert(12);
set.insert(9);
CHECK(set.size() == 6);

// erase a number
bool const erased = set.erase(8);
CHECK(erased);
CHECK(set.size() == 5);

// minimum and maximum
CHECK(set.min_key() == 1);
CHECK(set.max_key() == 12);
{ auto const r = set.min(); CHECK(r.exists); CHECK(r.key == 1); }
{ auto const r = set.max(); CHECK(r.exists); CHECK(r.key == 12); }

// membership queries
CHECK(set.contains(1));
CHECK(set.contains(5));
CHECK(set.contains(12));

CHECK(!set.contains(0));
CHECK(!set.contains(8)); // erased
CHECK(!set.contains(13));

// alternative membership queries
{ auto const r = set.find(1); CHECK(r.exists); CHECK(r.key == 1); }
{ auto const r = set.find(0); CHECK(!r.exists); }

// predecessor queries
{ auto const r = set.predecessor(0); CHECK(!r.exists); }
{ auto const r = set.predecessor(1); CHECK(r.exists); CHECK(r.key == 1); }
{ auto const r = set.predecessor(2);  CHECK(r.exists); CHECK(r.key == 1); }
{ auto const r = set.predecessor(13); CHECK(r.exists); CHECK(r.key == 12); }

// successor queries
{ auto const r = set.successor(0); CHECK(r.exists); CHECK(r.key == 1); }
{ auto const r = set.successor(1); CHECK(r.exists); CHECK(r.key == 1); }
{ auto const r = set.successor(2);  CHECK(r.exists); CHECK(r.key == 4); }
{ auto const r = set.successor(13); CHECK(!r.exists); }
```

### Ordered Maps

The associative variants `ordered::btree::Map` and `ordered::range_marking::Map` work just like the set counterparts, except you associate values to each key upon insertion and the [QueryResult](#QueryResult) returned by queries also contain the corresponding value.

##### Example

The following excerpt from the tests should tell you all you need to know about the usage.

```cpp
// start with an empty map
ordered::Map<int, int> map;
// alternatively, using maximum possible key 15
// ordered::range_marking::Map<unsigned int, unsigned int> map(15);

CHECK(map.empty());

// insert some numbers with associated values
map.insert(5, 500);
map.insert(1, 100);
map.insert(8, 800);
map.insert(4, 400);
map.insert(12, 1200);
map.insert(9, 900);
CHECK(map.size() == 6);

// erase a number
bool const erased = map.erase(8);
CHECK(erased);
CHECK(map.size() == 5);

// minimum and maximum
CHECK(map.min_key() == 1);
CHECK(map.max_key() == 12);
{ auto const r = map.min(); CHECK(r.exists); CHECK(r.key == 1);  CHECK(r.value == 100); }
{ auto const r = map.max(); CHECK(r.exists); CHECK(r.key == 12); CHECK(r.value == 1200); }

// membership queries
CHECK(map.contains(1));
CHECK(map.contains(5));
CHECK(map.contains(12));

CHECK(!map.contains(0));
CHECK(!map.contains(3));
CHECK(!map.contains(13));

// alternative membership queries / lookup
{ auto const r = map.find(1); CHECK(r.exists); CHECK(r.key == 1); CHECK(r.value == 100); }
{ auto const r = map.find(0); CHECK(!r.exists); }

// predecessor queries
{ auto const r = map.predecessor(0); CHECK(!r.exists); }
{ auto const r = map.predecessor(1); CHECK(r.exists); CHECK(r.key == 1); CHECK(r.value == 100); }
{ auto const r = map.predecessor(2);  CHECK(r.exists); CHECK(r.key == 1);  CHECK(r.value == 100); }
{ auto const r = map.predecessor(13); CHECK(r.exists); CHECK(r.key == 12); CHECK(r.value == 1200); }

// successor queries
{ auto const r = map.successor(0); CHECK(r.exists); CHECK(r.key == 1); CHECK(r.value == 100); }
{ auto const r = map.successor(1); CHECK(r.exists); CHECK(r.key == 1); CHECK(r.value == 100); }
{ auto const r = map.successor(2);  CHECK(r.exists); CHECK(r.key == 4);  CHECK(r.value == 400); }
{ auto const r = map.successor(13); CHECK(!r.exists); }
```

