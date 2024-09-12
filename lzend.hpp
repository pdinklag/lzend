/**
 * lzend.hpp
 * part of pdinklag/lzend
 * 
 * MIT License
 * 
 * Copyright (c) Patrick Dinklage
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

#ifndef _LZEND_HPP
#define _LZEND_HPP

#include "libsais.h"

#include <cstdint>
#include <iostream>
#include <functional>
#include <vector>

#include <rmq/rmq.hpp>
#include <ordered/btree/map.hpp>

namespace lzend {

using Index = int32_t;

// represents an LZ-End phrase
struct Phrase {
    Index lnk;
    Index len;
    char  ext;
};

// computes the LZ-End parsing and returns the number of phrases
std::vector<Phrase> parse(std::string const& s, bool print_progress = false) {
    Index const n = s.length();
    if(print_progress) std::cout << "LZ-End input: n=" << n << std::endl;

    // reverse text
    std::string rs = s;
    std::reverse(rs.begin(), rs.end());

    // construct suffix array
    if(print_progress) std::cout << "\tconstruct suffix array ..." << std::endl;
    auto sa = std::make_unique<Index[]>(n);
    libsais((uint8_t const*)rs.data(), sa.get(), n, 0, nullptr);
    
    // construct PLCP array and the LCP array from it
    if(print_progress) std::cout << "\tconstruct LCP array ..." << std::endl;
    auto isa = std::make_unique<Index[]>(n);
    auto& plcp = isa;
    libsais_plcp((uint8_t const*)rs.data(), sa.get(), isa.get(), n);
    
    auto lcp = std::make_unique<Index[]>(n);
    libsais_lcp(plcp.get(), sa.get(), lcp.get(), n);
    
    // construct RMQ data structure
    if(print_progress) std::cout << "\tconstruct RMQ ..." << std::endl;
    rmq::RMQ<Index> rmq(lcp.get(), n);
    
    // construct permuted inverse suffix array
    if(print_progress) std::cout << "\tconstruct permuted inverse suffix array ..." << std::endl;
    for(Index i = 0; i < n; i++) isa[n-sa[i]-1] = i;
    
    // discard suffix array and reverse text
    sa.reset();
    rs = std::string();
    
    // initialize predecessor/successor
    ordered::btree::Map<Index, Index> marked;
    
    // helpers
    struct Candidate { Index lex_pos; Index lnk; Index len; };
    
    auto lex_smaller_phrase = [&](Index const x){
        auto const r = marked.predecessor(x-1);
        return r.exists
            ? Candidate { r.key, r.value, lcp[rmq(r.key+1, x)] }
            : Candidate { 0, 0, 0 };
    };

    auto lex_greater_phrase = [&](Index const x){
        auto const r = marked.successor(x+1);
        return r.exists
            ? Candidate { r.key, r.value, lcp[rmq(x+1, r.key)] }
            : Candidate { 0, 0, 0 };
    };
    
    // parse
    std::cout << "\tparse ... ";
    std::cout.flush();
    
    std::vector<Phrase> parsing;
    parsing.push_back({0, 1, s[0]}); // initial empty phrase
    Index z = 0; // index of latest phrase (number of phrases would be z+1)
    
    for(Index i = 1; i < n; i++) {
        auto const len1 = parsing[z].len;
        auto const len2 = len1 + (z > 0 ? parsing[z-1].len : 0);
        
        auto const isa_last = isa[i-1]; // suffix array neighbourhood 
    
        // find source phrase candidates
        Index p1 = -1, p2 = -1;
        auto find_copy_source = [&](std::function<Candidate(Index)> f){
            auto c = f(isa_last);
            if(c.len >= len1) {
                p1 = c.lnk;
                if(i > len1) {
                    if(c.lnk == z-1) c = f(c.lex_pos);
                    if(c.len >= len2) p2 = c.lnk;
                }
            }
        };
    
        find_copy_source(lex_smaller_phrase);
        if(p1 == -1 || p2 == -1) {
            find_copy_source(lex_greater_phrase);
        }

        // case distinction according to Lemma 2
        if(p2 != -1) {
            // merge last two phrases
            marked.erase(isa[i - 1 - len1]);
            
            parsing.pop_back();
            --z;
            
            parsing.back() = Phrase { p2, len2 + 1, s[i] };
        } else if(p1 != -1) {
            // extend last phrase
            parsing.back() = Phrase { p1, len1 + 1, s[i] };
        } else {
            // lazily mark previous phrase
            marked.insert(isa_last, z);

            // begin new phrase
            parsing.push_back(Phrase{ 0, 1, s[i] });
            ++z;
        }
    }
    return parsing;
}

}

#endif
