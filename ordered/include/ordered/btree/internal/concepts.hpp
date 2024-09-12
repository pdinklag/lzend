/**
 * ordered/btree/internal/concepts.hpp
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

#ifndef _ORDERED_BTREE_INTERNAL_CONCEPTS_HPP
#define _ORDERED_BTREE_INTERNAL_CONCEPTS_HPP

#include <concepts>

#include "../../internal/local_query_result.hpp"

namespace ordered::btree::internal {

/**
 * \brief Concept for types that provide a B-Tree node implementation
 * 
 * In addition to satisfying the concept syntactically, the keys returned by `operator[]` must respect the total order of the keys,
 * e.g., `impl[i] < impl[j]` must hold for `i < j`.
 * 
 * \tparam T the type in question
 */
template<typename T>
concept BTreeNode =
    requires {
        typename T::Key;
        typename T::Value;
        { T::capacity() } -> std::unsigned_integral;
    }
    && std::totally_ordered<typename T::Key>
    && requires(T const& impl) {
        { impl.size() } -> std::unsigned_integral;
    }
    && requires(T& impl, typename T::Key const key) {
        { impl.erase(key) } -> std::same_as<bool>;
    }
    && requires(T const& impl, typename T::Key const key) {
        { impl.predecessor(key) } -> std::same_as<ordered::internal::LocalQueryResult>;
        { impl.successor(key) } -> std::same_as<ordered::internal::LocalQueryResult>;
    }
    && requires(T& impl, typename T::Key const key, typename T::Value& out_value) {
        { impl.erase(key, out_value) } -> std::same_as<bool>;
    }
    && requires(T& impl, typename T::Key const key, typename T::Value value) {
        { impl.insert(key, value) };
    }
    && requires(T const& impl, size_t const i) {
        { impl[i] } -> std::same_as<typename T::Key>;
        { impl.value(i) } -> std::same_as<typename T::Value>;
    };

}

#endif
