# LZ-End

This repository contains a minimalistic state-of-the-art C++20 implementation of the LZ-End compression scheme.

> The LZ-End parsing [Kreft & Navarro, 2011] of an input string yields compression competitive with the popular Lempel-Ziv 77 scheme, but also allows for efficient random access. Kempa and Kosolobov showed that the parsing can be computed in time and space linear in the input length [Kempa & Kosolobov, 2017], however, the corresponding algorithm is hardly practical. We put the spotlight on their suboptimal algorithm that computes the parsing in time O(n lglg n). It requires a comparatively small toolset and is therefore easy to implement, but at the same time very efficient in practice. We give a detailed and simplified description with a full listing that incorporates undocumented tricks from the original implementation, but also uses lazy evaluation to reduce the workload in practice and requires less working memory by removing a level of indirection. We legitimize our algorithm in a brief benchmark, obtaining the parsing faster than the state of the art.

A detailed description of the algorithm is available in the paper *[Computing the LZ-End parsing: Easy to implement and practically efficient](https://arxiv.org/abs/2409.07840)* that is freely available on arXiv.

## Usage

The implementation is meant to serve as a didactic resource for reference or possibly a library that you may adopt for your own use case. It is *not* meant to provide a compression tool.

The function `lzend::parse` (defined in `lzend.hpp`, which you may include) computes the LZ-End parsing of the input string. The implementation is very close to the pseucode given in the aforementioned paper, which explains it in detail. Its return type of the function is a `std::vector` of phrases stored in a simple struct, and thus phrases are not stored in a succinct manner.

You can use the parsing to construct a succinct access data structure or encode the parsing for compression. However, none of that is provided in this repository, and neither are any means to decode the parsing or do random access on the original input.

## Building

The provided `Makefile` can be used to build the code using `g++` by simply executing `make`. The code uses C++20 features and thus requires use of a suitable compiler.

## License

> MIT License
>
> Copyright (c) Patrick Dinklage
>
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.