/**
 * ordered/btree/internal/linear_search_map.hpp
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

#ifndef _ORDERED_BTREE_INTERNAL_LINEAR_SEARCH_MAP_HPP
#define _ORDERED_BTREE_INTERNAL_LINEAR_SEARCH_MAP_HPP

#include "linear_search_base.hpp"

namespace ordered::btree::internal {

/**
 * \brief B-tree node implementation that stores keys naively in a sorted array
 * 
 * The corresponding values are stored in another array with the same layout.
 * 
 * Let N be the node's capacity, then the asymptotic search, insert and removal times are O(N) each.
 * The intention is that N is chosen to be reasonably small.
 * 
 * \tparam Key the key type
 * \tparam Value_ the value type
 * \tparam capacity_ the node's capacity
 */
template<std::totally_ordered Key_, typename Value_, size_t capacity_>
class LinearSearchMap : public LinearSearchBase<Key_, capacity_> {
public:
    static constexpr size_t capacity() { return capacity_; }
    using Key = Key_;
    using Value = Value_;

private:
    Value values_[capacity_];

public:
    LinearSearchMap() {
    }

    LinearSearchMap(const LinearSearchMap&) = default;
    LinearSearchMap(LinearSearchMap&&) = default;

    LinearSearchMap& operator=(const LinearSearchMap&) = default;
    LinearSearchMap& operator=(LinearSearchMap&&) = default;

    inline Value value(size_t const i) const {
        return values_[i];
    }

    inline void insert(Key const key, Value const value) {
        auto const old_size = this->size();
        auto const i = this->insert_key(key);

        for(size_t j = old_size; j > i; j--) values_[j] = values_[j-1];
        values_[i] = value;
    }

    inline bool erase(Key const key, Value& value) {
        size_t i;
        if(this->erase_key(key, i)) {
            auto const new_size = this->size();
            value = values_[i];
            for(size_t j = i; j < new_size; j++) values_[j] = values_[j+1];
            return true;
        } else {
            return false;
        }
    }

    inline bool erase(Key const key) {
        Value discard;
        return erase(key, discard);
    }
} __attribute__((__packed__));

}

#endif
