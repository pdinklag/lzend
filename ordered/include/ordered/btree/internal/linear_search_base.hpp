/**
 * ordered/btree/internal/linear_search_base.hpp
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

#ifndef _ORDERED_BTREE_INTERNAL_LINEAR_SEARCH_BASE_HPP
#define _ORDERED_BTREE_INTERNAL_LINEAR_SEARCH_BASE_HPP

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "../../internal/local_query_result.hpp"

namespace ordered::btree::internal {

/**
 * \brief Base for B-tree node implementations that store keys naively in a sorted array
 * 
 * Let N be the node's capacity, then the asymptotic search, insert and removal times are O(N) each.
 * The intention is that N is chosen to be reasonably small.
 * 
 * \tparam Key the key type
 * \tparam capacity_ the node's capacity
 */
template<std::totally_ordered Key, size_t capacity_>
class LinearSearchBase {
private:
    static_assert(capacity_ < 65536);
    using Size = typename std::conditional_t<capacity_ < 256, uint8_t, uint16_t>;

    Key keys_[capacity_];
    Size size_;

protected:
    inline size_t insert_key(Key const key) {
        assert(size_ < capacity_);
        size_t i = 0;
        while(i < size_ && keys_[i] < key) ++i;
        for(size_t j = size_; j > i; j--) keys_[j] = keys_[j-1];
        keys_[i] = key;

        ++size_;
        return i;
    }

    inline bool erase_key(Key const key, size_t& pos) {
        assert(size_ > 0);
        for(size_t i = 0; i < size_; i++) {
            if(keys_[i] == key) {
                pos = i;
                for(size_t j = i; j < size_-1; j++) keys_[j] = keys_[j+1];

                --size_;
                return true;
            }
        }
        return false;
    }

public:
    LinearSearchBase(): size_(0) {
    }

    LinearSearchBase(const LinearSearchBase&) = default;
    LinearSearchBase(LinearSearchBase&&) = default;

    LinearSearchBase& operator=(const LinearSearchBase&) = default;
    LinearSearchBase& operator=(LinearSearchBase&&) = default;

    inline Key operator[](size_t const i) const {
        return keys_[i];
    }

    inline ordered::internal::LocalQueryResult predecessor(Key const x) const {
        if(size_ == 0) [[unlikely]] return { false, 0 };
        if(x < keys_[0]) [[unlikely]] return { false, 0 };
        if(x >= keys_[size_-1]) [[unlikely]] return { true, size_ - 1ULL };
        
        size_t i = 1;
        while(keys_[i] <= x) ++i;
        return { true, i-1 };
    }

    inline ordered::internal::LocalQueryResult successor(Key const x) const {
        if(size_ == 0) [[unlikely]] return { false, 0 };
        if(x <= keys_[0]) [[unlikely]] return { true, 0 };
        if(x > keys_[size_-1]) [[unlikely]] return { false, 0 };
        
        size_t i = 1;
        while(keys_[i] < x) ++i;
        return { true, i };
    }

    inline size_t size() const { return size_; }
} __attribute__((__packed__));

}

#endif
