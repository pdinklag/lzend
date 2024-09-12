/**
 * test_examples.cpp
 * part of pdinklag/code
 * 
 * MIT License
 * 
 * Copyright (c) 2023 Patrick Dinklage
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <ordered/btree.hpp>
#include <ordered/range_marking.hpp>

namespace ordered::test {

template<typename Set>
void test_set(Set& set) {
    // start with an empty set
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
}

template<typename Map>
void test_map(Map& map) {
    // start with an empty map
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
}

TEST_SUITE("ordered") {
    TEST_CASE("btree::Set") {
        ordered::btree::Set<int> set;
        test_set(set);
    }

    TEST_CASE("btree::Map") {
        ordered::btree::Map<int, int> map;
        test_map(map);
    }

    TEST_CASE("range_marking::Set") {
        ordered::range_marking::Set<unsigned int> set(15);
        test_set(set);
    }

    TEST_CASE("range_marking::Map") {
        ordered::range_marking::Map<unsigned int, unsigned int> map(15);
        test_map(map);
    }
}

}
