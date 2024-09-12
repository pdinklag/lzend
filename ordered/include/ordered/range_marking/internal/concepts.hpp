/**
 * ordered/range_marking/concepts.hpp
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

#ifndef _ORDERED_RANGE_MARKING_CONCEPTS_HPP
#define _ORDERED_RANGE_MARKING_CONCEPTS_HPP

#include <concepts>
#include <limits>

#include "../../internal/local_query_result.hpp"

namespace ordered::range_marking::internal {

template<typename T>
concept RangeMarkBucket =
    requires {
        typename T::Index;
        typename T::Value;
        { T::capacity() } -> std::unsigned_integral;
    }
    && std::unsigned_integral<typename T::Key>
    && std::unsigned_integral<typename T::Index>
    && std::numeric_limits<typename T::Index>::digits <= std::numeric_limits<typename T::Key>::digits
    && requires(typename T::Key const key) {
        { T::to_index(key) } -> std::same_as<typename T::Index>;
    }
    && requires(T const& impl) {
        { impl.size() } -> std::unsigned_integral;
    }
    && requires(T& bucket, typename T::Index const index) {
        { bucket.erase(index) } -> std::same_as<bool>;
    }
    && requires(T const& bucket) {
        { bucket.min() } -> std::same_as<typename T::Index>;
        { bucket.max() } -> std::same_as<typename T::Index>;
    }
    && requires(T const& bucket, typename T::Index const x) {
        { bucket.predecessor(x) } -> std::same_as<ordered::internal::LocalQueryResult>;
        { bucket.successor(x) } -> std::same_as<ordered::internal::LocalQueryResult>;
    }
    && requires(T& bucket, typename T::Index const x, typename T::Value value) {
        { bucket.insert(x, value) };
    }
    && requires(T const& bucket, typename T::Index const x) {
        { bucket.value(x) } -> std::same_as<typename T::Value>;
    };
}

#endif
