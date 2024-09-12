/**
 * test_static.cpp
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

#include <cstdint>
#include <limits>

#include <ordered/btree/internal/linear_search_map.hpp>
#include <ordered/btree/internal/linear_search_set.hpp>

constexpr size_t small = 255;
constexpr size_t large = 256;

using Key = uint32_t;
using Value = uint32_t;

constexpr size_t key_size = sizeof(Key);
constexpr size_t value_size = sizeof(Value);

static_assert(sizeof(ordered::btree::internal::LinearSearchSet<Key, small>) == (small * key_size + sizeof(uint8_t)));
static_assert(sizeof(ordered::btree::internal::LinearSearchSet<Key, large>) == (large * key_size + sizeof(uint16_t)));

static_assert(sizeof(ordered::btree::internal::LinearSearchMap<Key, Value, small>) == (small * (key_size + value_size) + sizeof(uint8_t)));
static_assert(sizeof(ordered::btree::internal::LinearSearchMap<Key, Value, large>) == (large * (key_size + value_size) + sizeof(uint16_t)));

int main() {
    // we compiled, so we passed
    return 0;
}
