/**
 * ordered/range_marking/bucket_base.hpp
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

#ifndef _ORDERED_RANGE_MARKING_BUCKET_BASE_HPP
#define _ORDERED_RANGE_MARKING_BUCKET_BASE_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <concepts>
#include <limits>
#include <memory>
#include <type_traits>

#include "idiv_ceil.hpp"
#include "../../internal/local_query_result.hpp"

namespace ordered::range_marking::internal {

/**
 * \brief Base for range-marking buckets
 * 
 * A bucket contains marks for a certain key range, stored in a bit vector.
 * Insertion and deletion involves setting or clearing a bit, respectively.
 * Queries are answered using linear scans over the bits.
 * 
 * \tparam Key the type
 * \tparam capacity_ the bucket capacity; must be a power of two
 */
template<std::unsigned_integral Key, size_t capacity_>
class BucketBase {
public:
    static_assert(capacity_ <= UINT32_MAX);
    static_assert(std::has_single_bit(capacity_), "bucket capacity must be a power of two");

    using Index = std::conditional_t<capacity_ <= UINT8_MAX, uint8_t, std::conditional_t<capacity_ <= UINT16_MAX, uint16_t, uint32_t>>;

    static constexpr Index to_index(Key const key) {
        return Index(key % capacity_);
    }

private:
    using Pack = uint64_t;
    static constexpr size_t PACK_BITS = std::numeric_limits<Pack>::digits;
    static constexpr size_t num_packs_ = idiv_ceil(capacity_, PACK_BITS);

    static_assert(capacity_ >= PACK_BITS);

private:
    static constexpr uint8_t lowest_set_bit(Pack const x) {
        return std::countr_zero(x);
    }

    static constexpr uint8_t highest_set_bit(Pack const x) {
        return PACK_BITS - 1 - std::countl_zero(x);
    }

    Index size_;
    std::unique_ptr<Pack[]> data_;

protected:
    bool get_bit(size_t const i) const {
        size_t const a = i / PACK_BITS;
        size_t const j = i % PACK_BITS;
        size_t const mask = 1ULL << j;
        return (data_[a] & mask) ? 1 : 0;
    }

    void set_bit(size_t const i, bool const b) {
        size_t const a = i / PACK_BITS;
        size_t const j = i % PACK_BITS;
        size_t const mask = 1ULL << j;
        data_[a] = (data_[a] & ~mask) | (-b & mask); // nb: clear and set conditionally
    }

public:
    inline BucketBase() : size_(0), data_(std::make_unique<uint64_t[]>(num_packs_)) {
        for(size_t i = 0; i < num_packs_; i++) data_[i] = 0;
    }

    BucketBase(BucketBase&&) = default;
    BucketBase& operator=(BucketBase&&) = default;
    BucketBase(BucketBase const&) = delete;
    BucketBase& operator=(BucketBase const&) = delete;

    inline size_t size() const {
        return size_;
    }

    inline void insert(Index const x) {
        set_bit(x, 1);
        ++size_; // nb: we do NOT check whether the key was already contained, it is documented that duplicate keys are to cause undefined behaviour
    }

    inline bool erase(Index const x) {
        auto const b = get_bit(x);
        set_bit(x, 0);
        if(b) --size_;
        return b;
    }

    bool contains(Index const x) const {
        return get_bit(x);
    }

    inline ordered::internal::LocalQueryResult predecessor(Index const x) const {
        auto const* data = data_.get();
        auto i = x / PACK_BITS;

        if(data[i]) {
            // the predecessor may be in x's own pack word
            auto j = x % PACK_BITS;
            auto m = 1ULL << j;

            auto const pack = data[i];
            while(m) {
                if(pack & m) return { true, i * PACK_BITS + j };

                m >>= 1;
                --j;
            }
        }

        // we did not find a predecesor in the corresponding word
        // thus, the predecessor is the highest set previous bit
        while(i > 0) {
            if(data[i-1] != 0) {
                // find the last set bit
                return { true, (i-1) * PACK_BITS + highest_set_bit(data[i-1]) };
            }
            --i;
        }
        return ordered::internal::LocalQueryResult::none();
    }

    inline ordered::internal::LocalQueryResult successor(Index const x) const {
        auto const* data = data_.get();
        auto i = x / PACK_BITS;

        if(data[i]) {
            // the successor may be in x's own pack word
            auto j = x % PACK_BITS;
            auto m = 1ULL << j;

            auto const pack = data[i];
            while(j < PACK_BITS) {
                if(pack & m) return { true, i * PACK_BITS + j };

                m <<= 1;
                ++j;
            }
        }

        // we did not find a successor in the corresponding word
        // thus, the successor is the lowest set next bit
        ++i;
        while(i < num_packs_) {
            if(data[i] != 0) {
                // find the last set bit
                return { true, i * PACK_BITS + lowest_set_bit(data[i]) };
            }
            ++i;
        }
        return ordered::internal::LocalQueryResult::none();
    }

    inline Index min() const {
        auto const* data = data_.get();

        // find first pack that has a set bit
        for(size_t i = 0; i < num_packs_; i++) {
            if(data[i] != 0) {
                // find the first set bit
                return i * PACK_BITS + lowest_set_bit(data[i]);
            }
        }

        assert(false); // there must not be any empty buckets
        return Index();
    }

    inline Index max() const {
        auto const* data = data_.get();

        // find last pack that has a set bit
        for(size_t i = num_packs_; i > 0; i--) {
            if(data[i-1] != 0) {
                // find the last set bit
                return (i-1) * PACK_BITS + highest_set_bit(data[i-1]);
            }
        }

        assert(false); // there must not be any empty buckets
        return Index();
    }
};

}

#endif
