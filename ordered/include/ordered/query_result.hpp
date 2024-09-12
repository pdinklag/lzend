/**
 * ordered/query_result.hpp
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

#ifndef _ORDERED_QUERY_RESULT_HPP
#define _ORDERED_QUERY_RESULT_HPP

namespace ordered {

/**
 * \brief The result of a search query
 * 
 * \tparam Key the key type
 * \tparam Value the value type
 */
template<typename Key, typename Value>
struct QueryResult {
    /**
     * \brief Produces a negative query result
     * 
     * \return a negative query result 
     */
    inline static QueryResult none() { return { false, Key(), Value() }; }

    /**
     * \brief Tells whether or not the predecessor or successor exists
     */
    bool exists;

    /**
     * \brief The predecessor or successor if it exists, undefined otherwise
     */
    Key key;

    /**
     * \brief The value associated with the predecessor or successor if it exists, undefined otherwise
     * 
     * The value type depends on the node implementation.
     * For data structures that represent a set (only storing keys and no values), this may be a dummy.
     */
    Value value;

    inline operator bool() const { return exists; }
};

}

#endif
