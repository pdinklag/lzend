/**
 * ordered/internal/local_query_result.hpp
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

#ifndef _ORDERED_INTERNAL_LOCAL_QUERY_RESULT_HPP
#define _ORDERED_INTERNAL_LOCAL_QUERY_RESULT_HPP

#include <cstddef>

namespace ordered::internal {

/**
 * \brief The result of a node- or bucket-local query
 */
struct LocalQueryResult {
    /**
     * \brief Produces a negative query result
     * 
     * \return a negative query result 
     */
    inline static LocalQueryResult none() { return { false, 0 }; }

    /**
     * \brief Tells whether or not a query result exists
     */
    bool exists;

    /**
     * \brief The position of the query result if it exists, undefined otherwise
     */
    size_t pos;

    inline operator bool() const { return exists; }
    inline operator size_t() const { return pos; }
};

}

#endif
