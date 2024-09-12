/**
 * lzend.cpp
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

#include <algorithm>
#include <cassert>
#include <chrono>

#include <fstream>
#include <memory>

#include "lzend.hpp"

uintmax_t timestamp() {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "usage: " << argv[0] << " [FILE]" << std::endl;
        return -1;
    }
    
    // load input file
    std::string s;
    {
        std::ifstream ifs(argv[1]);
        s = std::string(std::istreambuf_iterator<char>(ifs), {});
    }
    
    // parse
    auto const t0 = timestamp();
    auto const z = lzend::parse(s, true).size();
    auto const dt = timestamp() - t0;
    std::cout << "-> z=" << z << " (" << dt << " ms)" << std::endl;
    return 0;
}
