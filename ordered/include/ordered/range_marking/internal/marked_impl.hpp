/**
 * ordered/range_marking/marked_impl.hpp
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

#ifndef _ORDERED_RANGE_MARKING_IMPL_HPP
#define _ORDERED_RANGE_MARKING_IMPL_HPP

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

#include "concepts.hpp"
#include "idiv_ceil.hpp"
#include "../../query_result.hpp"

namespace ordered::range_marking::internal {

/**
 * \brief A range-marking data structure
 * 
 * The universe of keys (stated on construction) is partitioned into ranges of size defined by the given bucket implementation.
 * Operations are then delegated to the corresponding bucket.
 * 
 * Only active (non-empty) buckets are actually held in memory.
 * Minimum, maximum, predecessor and successor queries may involve linear scans for a suitable active bucket.
 * 
 * Attempts to insert, remove or query for keys beyond the initially stated maximum key will result in undefined behaviour.
 * 
 * \tparam Bucket the bucket implementation
 */
template<RangeMarkBucket BucketImpl>
class RangeMarker {
private:
    static constexpr size_t sampling_ = BucketImpl::capacity();

    using Key = BucketImpl::Key;
    using Index = BucketImpl::Index;
    using Value = BucketImpl::Value;

    static constexpr Key to_key(Index const i, size_t const bucket_num) {
        return bucket_num * sampling_ + i;
    }

    static constexpr Key bucket_for(Key const key) {
        return key / sampling_;
    }

    size_t num_buckets_;
    std::unique_ptr<BucketImpl*[]> buckets_;
    Key max_bucket_num_;
    size_t size_;

    size_t min_bucket() const {
        for(size_t i = 0; i <= max_bucket_num_; i++) {
            if(buckets_[i]) return i;
        }
        assert(false);
        return -1;
    }

    size_t max_bucket() const {
        for(size_t i = max_bucket_num_ + 1; i > 0; i--) {
            if(buckets_[i - 1]) return i - 1;
        }
        assert(false);
        return -1;
    }

public:
    /**
     * \brief Construct an empty container
     * 
     * \param max_key the maximum possible key to be inserted into the container
     */
    RangeMarker(Key const max_key)
        : num_buckets_(idiv_ceil(max_key, sampling_)),
          buckets_(std::make_unique<BucketImpl*[]>(num_buckets_)),
          max_bucket_num_(0),
          size_(0) {

        for(size_t i = 0; i < num_buckets_; i++) buckets_[i] = nullptr;
    }

    ~RangeMarker() {
        clear();
    }

    /**
     * \brief Clears the container
     */
    void clear() {
        for(size_t i = 0; i < num_buckets_; i++) {
            if(buckets_[i]) {
                delete buckets_[i];
                buckets_[i] = nullptr;
            }
        }
        max_bucket_num_ = 0;
    }

    /**
     * \brief Inserts the given key and associated value
     * 
     * Inserting a key that is already contained results in undefined behaviour.
     * 
     * \param key the key to insert; must be at most the maximum key stated during construction
     * \param value the value to associate with the key
     */
    void insert(Key const key, Value const value) {
        auto const bucket_num = bucket_for(key);
        assert(bucket_num < num_buckets_);

        BucketImpl* bucket = buckets_[bucket_num];
        if(!bucket) {
            bucket = new BucketImpl();
            buckets_[bucket_num] = bucket;
            max_bucket_num_ = std::max(bucket_num, max_bucket_num_);
        }

        bucket->insert(BucketImpl::to_index(key), value);
        ++size_;
    }

    /**
     * \brief Inserts the given key
     * 
     * The associated value will is default-constructed.
     * Inserting a key that is already contained results in undefined behaviour.
     * 
     * \param key the key to insert; must be at most the maximum key stated during construction
     */
    inline void insert(Key const key) {
        insert(key, Value{});
    }

    /**
     * \brief Removes the given key
     * 
     * \param key the key to remove; must be at most the maximum key stated during construction
     * \return true if the key was contained and has been removed by the operation
     * \return false if the key was not contained and thus nothing was removed
     */
    bool erase(Key const key) {
        auto const bucket_num = bucket_for(key);
        assert(bucket_num < num_buckets_);

        BucketImpl* bucket = buckets_[bucket_num];
        if(bucket) [[likely]] {
            if(bucket->erase(BucketImpl::to_index(key))) {
                --size_;
                if(bucket->size() == 0) {
                    // delete empty bucket
                    delete bucket;
                    buckets_[bucket_num] = nullptr;
                }
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    /**
     * \brief Finds the predecessor of the given key, if any
     * 
     * If the key is contained, it will be returned as its own predecessor.
     * 
     * \param key the key in question; must be at most the maximum key stated during construction
     * \return the query result
     */
    QueryResult<Key, Value> predecessor(Key const key) const {
        auto bucket_num = bucket_for(key);
        assert(bucket_num < num_buckets_);

        // find predecessor in corresponding bucket
        {
            BucketImpl const* bucket = buckets_[bucket_num];
            if(bucket) {
                // we found the right bucket, attempt to find the predecessor in there
                auto r = bucket->predecessor(BucketImpl::to_index(key));
                if(r.exists) {
                    return { true, to_key(r.pos, bucket_num), bucket->value(r.pos) };
                }
            }
        }
        
        // we either couldn't find the right bucket, or the predecessor was not in there
        // find the preceding bucket and return its maximum
        while(bucket_num) {
            --bucket_num;
            BucketImpl const* bucket = buckets_[bucket_num];
            if(bucket) {
                auto const max = bucket->max();
                return { true, to_key(max, bucket_num), bucket->value(max) };
            }
        }

        // nope
        return QueryResult<Key, Value>::none();
    }

    /**
     * \brief Finds the successor of the given key, if any
     * 
     * If the key is contained, it will be returned as its own successor.
     * 
     * \param key the key in question; must be at most the maximum key stated during construction
     * \return the query result
     */
    QueryResult<Key, Value> successor(Key const key) const {
        auto bucket_num = bucket_for(key);
        assert(bucket_num < num_buckets_);

        // find successor in corresponding bucket
        {
            BucketImpl const* bucket = buckets_[bucket_num];
            if(bucket) {
                // we found the right bucket, attempt to find the successor in there
                auto r = bucket->successor(BucketImpl::to_index(key));
                if(r.exists) {
                    return { true, to_key(r.pos, bucket_num), bucket->value(r.pos) };
                }
            }
        }
        
        // we either couldn't find the right bucket, or the successor was not in there
        // find the succeeding bucket and return its minimum
        while(bucket_num < max_bucket_num_) {
            ++bucket_num;
            BucketImpl const* bucket = buckets_[bucket_num];
            if(bucket) {
                auto const min = bucket->min();
                return { true, to_key(min, bucket_num), bucket->value(min) };
            }
        }

        // nope
        return QueryResult<Key, Value>::none();
    }

    /**
     * \brief Tests whether the given key is contained
     * 
     * \param key the key in question; must be at most the maximum key stated during construction
     * \return true iff the key is contained
     * \return false otherwise
     */
    bool contains(Key const key) const {
        auto bucket_num = bucket_for(key);
        assert(bucket_num < num_buckets_);

        BucketImpl const* bucket = buckets_[bucket_num];
        if(bucket) {
            return bucket->contains(BucketImpl::to_index(key));
        } else {
            return false;
        }
    }

    /**
     * \brief Finds the given key
     * 
     * \param key the key in question; must be at most the maximum key stated during construction
     * \return the query result
     */
    inline QueryResult<Key, Value> find(Key const key) const {
        auto bucket_num = bucket_for(key);
        assert(bucket_num < num_buckets_);

        BucketImpl const* bucket = buckets_[bucket_num];
        auto const x = BucketImpl::to_index(key);
        if(bucket && bucket->contains(x)) {
            return { true, key, bucket->value(x) };
        } else {
            return QueryResult<Key, Value>::none();
        }
    }


    /**
     * \brief Reports the minimum key contained
     * 
     * Querying this on an empty container results in undefined behaviour
     * 
     * \return the minimum key contained
     */
    inline Key min_key() const {
        auto const bucket_num = min_bucket();
        return to_key(buckets_[bucket_num]->min(), bucket_num);
    }

    /**
     * \brief Reports the maximum key contained
     * 
     * Querying this on an empty container results in undefined behaviour
     * 
     * \return the maximum key contained
     */
    inline Key max_key() const {
        auto const bucket_num = max_bucket();
        return to_key(buckets_[bucket_num]->max(), bucket_num);
    }

    /**
     * \brief Reports the minimum key contained and the associated value, if any
     * 
     * \return the minimum contained
     */
    inline QueryResult<Key, Value> min() const {
        if(size() == 0) [[unlikely]] return QueryResult<Key, Value>::none();

        auto const bucket_num = min_bucket();
        auto const* bucket = buckets_[bucket_num];
        auto const min = bucket->min();
        return { true, to_key(min, bucket_num), bucket->value(min) };
    }

    /**
     * \brief Reports the minimum key contained and the associated value, if any
     *
     * \return the maximum contained
     */
    inline QueryResult<Key, Value> max() const {
        if(size() == 0) [[unlikely]] return QueryResult<Key, Value>::none();

        auto const bucket_num = max_bucket();
        auto const* bucket = buckets_[bucket_num];
        auto const max = bucket->max();
        return { true, to_key(max, bucket_num), bucket->value(max) };
    }

    /**
     * \brief Reports the number of keys contained
     * 
     * \return the number of keys contained 
     */
    inline size_t size() const {
        return size_;
    }

    /**
     * \brief Reports whether the container is empty
     * 
     * \return true iff the container's size is zero
     * \return false iff there are any keys contained
     */
    inline bool empty() const {
        return size_ == 0;
    }
};

}

#endif
