/**
 * ordered/range_marking/bucket_map.hpp
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

#ifndef _ORDERED_RANGE_MARKING_BUCKET_MAP_HPP
#define _ORDERED_RANGE_MARKING_BUCKET_MAP_HPP

#include "bucket_base.hpp"

namespace ordered::range_marking::internal {

/**
 * \brief Associative range-marking bucket
 * 
 * A bucket contains marks for a certain key range, stored in a bit vector.
 * Insertion and deletion involves setting or clearing a bit, respectively.
 * Queries are answered using linear scans over the bits.
 * Values are stored in a simple array.
 * 
 * \tparam Key_ the type
 * \tparam Value_ the value type
 * \tparam capacity_ the bucket capacity; must be a power of two
 */
template<std::unsigned_integral Key_, typename Value_, size_t capacity_>
class BucketMap : public BucketBase<Key_, capacity_> {
private:
    using Base = BucketBase<Key_, capacity_>;

public:
    static constexpr size_t capacity() { return capacity_; }
    using Key = Key_;
    using Index = Base::Index;
    using Value = Value_;

private:
    std::unique_ptr<Value[]> values_;

public:
    inline BucketMap() : values_(std::make_unique<Value[]>(capacity_)) {
    }

    BucketMap(BucketMap&&) = default;
    BucketMap& operator=(BucketMap&&) = default;
    BucketMap(BucketMap const&) = delete;
    BucketMap& operator=(BucketMap const&) = delete;

    inline void insert(Index const x, Value const value) {
        Base::insert(x);
        values_[x] = value;
    }

    inline Value value(Index const x) const {
        return values_[x];
    }
};

}

#endif
