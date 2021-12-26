// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <https://unlicense.org>

// Mark Whitney 2021

#include "windows.h"
#include <iostream>

#include "SeqMatch.h"
#include "WinExTmr.h"


void dump(const std::vector<char>& rv1, std::vector<char>& rv2)
{
    std::cout << "  ";
    for (const auto& c1 : rv1)
    {
        std::cout << c1;
    }
    std::cout << std::endl;

    for (const auto& c2 : rv2)
    {
        std::cout << c2 << " ";
        for (const auto& c1 : rv1)
        {
            std::cout << ((c1 == c2) ? '@' : '+');
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


void test_random_char(
    const unsigned int seed,
    const int sym_ct,
    const size_t n1,
    const size_t n2,
    const bool is_dump = false)
{
    std::cout << "-------------------" << std::endl;
    std::cout << " SYM#=" << sym_ct;
    std::cout << " n1=" << n1;
    std::cout << " n2=" << n2;
    std::cout << std::endl;

    srand(seed);

    sequtil::SeqMatch<char> seqx;
    std::vector<char> v1(n1);
    std::vector<char> v2(n2);

    for (size_t ii = 0; ii < n1; ii++) v1[ii] = 'A' + (rand() % sym_ct);
    for (size_t ii = 0; ii < n2; ii++) v2[ii] = 'A' + (rand() % sym_ct);

    if (is_dump)
    {
        dump(v1, v2);
    }

    {
        // map (tree)
        WinExTmr extmr;
        sequtil::T_MAP_SZ2PT result;
        extmr.start();
        seqx.run_max(v1, v2, result);
        extmr.stop();
        std::cout << extmr.elapsed_time() << std::endl;

        std::cout << "Max Map Sz = " << seqx.max_map_size << std::endl;
        const auto& qlast = result.crbegin();
        std::cout << "Max Length = " << qlast->first << std::endl;
        for (const auto qpt : qlast->second)
        {
            std::cout << qpt.row << "," << qpt.col << std::endl;
        }
    }

    {
        // unordered map (hash)
        WinExTmr extmr;
        sequtil::T_MAP_SZ2PT result;
        extmr.start();
        seqx.run_max2(v1, v2, result);
        extmr.stop();
        std::cout << extmr.elapsed_time() << std::endl;

        std::cout << "Max Map Sz = " << seqx.max_map_size << std::endl;
        const auto& qlast = result.crbegin();
        std::cout << "Max Length = " << qlast->first << std::endl;
        for (const auto qpt : qlast->second)
        {
            std::cout << qpt.row << "," << qpt.col << std::endl;
        }
    }
}


void test_str(const std::string& rs1, const std::string& rs2)
{
    sequtil::SeqMatch<char> seqx;
    std::vector<char> v1(rs1.size());
    std::vector<char> v2(rs2.size());
    std::copy(rs1.begin(), rs1.end(), v1.begin());
    std::copy(rs2.begin(), rs2.end(), v2.begin());
    sequtil::T_MAP_SZ2PT result;
    seqx.run_all(v1, v2, result);
    dump(v1, v2);
}


int main()
{
    std::cout << "Longest Common Sequence Finder\n";

    test_str(
        "BBCCAAAAABBAAAA",
        "AAAAACCBBBBAAA");

    test_random_char(12345, 2, 100, 24, true);

    int sym_ct = 20;
    test_random_char(12345, sym_ct, 1000, 1000);
    test_random_char(12345, sym_ct, 10000, 1000);
    test_random_char(12345, sym_ct, 10000, 10000);
    test_random_char(12345, sym_ct, 100000, 10000);

    return 0;
}
