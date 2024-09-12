/**
 * ordered/btree/internal/linear_search_set.hpp
 * part of pdinklag/ordered
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

#ifndef _ORDERED_BTREE_INTERNAL_LINEAR_SEARCH_SET_HPP
#define _ORDERED_BTREE_INTERNAL_LINEAR_SEARCH_SET_HPP

#include "linear_search_base.hpp"

namespace ordered::btree::internal {

/**
 * \brief B-tree node implementation that stores keys naively in a sorted array
 * 
 * Let N be the node's capacity, then the asymptotic search, insert and removal times are O(N) each.
 * The intention is that N is chosen to be reasonably small.
 * 
 * \tparam Key the key type
 * \tparam capacity_ the node's capacity
 */
template<std::totally_ordered Key_, size_t capacity_>
class LinearSearchSet : public LinearSearchBase<Key_, capacity_> {
public:
    static constexpr size_t capacity() { return capacity_; }
    using Key = Key_;
    struct Value{};

    LinearSearchSet() {
    }

    LinearSearchSet(const LinearSearchSet&) = default;
    LinearSearchSet(LinearSearchSet&&) = default;

    LinearSearchSet& operator=(const LinearSearchSet&) = default;
    LinearSearchSet& operator=(LinearSearchSet&&) = default;

    inline Value value(size_t const i) const {
        return Value{};
    }

    inline void insert(Key const key) {
        this->insert_key(key);
    }

    inline void insert(Key const key, Value const value) {
        this->insert_key(key);
    }

    inline bool erase(Key const key, Value& value) {
        return this->erase(key);
    }

    inline bool erase(Key const key) {
        size_t discard;
        return this->erase_key(key, discard);
    }
} __attribute__((__packed__));

}

#endif
