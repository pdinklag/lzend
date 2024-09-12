/**
 * ordered/range_marking/bucket_set.hpp
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

#ifndef _ORDERED_RANGE_MARKING_BUCKET_SET_HPP
#define _ORDERED_RANGE_MARKING_BUCKET_SET_HPP

#include "bucket_base.hpp"

namespace ordered::range_marking::internal {

/**
 * \brief Range-marking bucket
 * 
 * A bucket contains marks for a certain key range, stored in a bit vector.
 * Insertion and deletion involves setting or clearing a bit, respectively.
 * Queries are answered using linear scans over the bits.
 * 
 * \tparam Key_ the type
 * \tparam capacity_ the bucket capacity; must be a power of two
 */
template<std::unsigned_integral Key_, size_t capacity_>
class BucketSet : public BucketBase<Key_, capacity_> {
private:
    using Base = BucketBase<Key_, capacity_>;

public:
    static constexpr size_t capacity() { return capacity_; }
    using Key = Key_;
    using Index = Base::Index;
    struct Value{};

    inline BucketSet() {}
    BucketSet(BucketSet&&) = default;
    BucketSet& operator=(BucketSet&&) = default;
    BucketSet(BucketSet const&) = delete;
    BucketSet& operator=(BucketSet const&) = delete;

    inline void insert(Index const x, Value const value) {
        Base::insert(x);
    }

    inline Value value(Index const x) const {
        return Value{};
    }
};

}

#endif
