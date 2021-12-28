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


void dump_grid(const std::vector<char>& rv1, std::vector<char>& rv2)
{
    int ii;
    std::cout << std::endl;

    ii = 0;
    std::cout << "   ";
    for (const auto& c1 : rv1)
    {
        char cc = '0' + (ii % 10);
        if (cc == '0') cc = '.';
        std::cout << cc;
        ii++;
    }
    std::cout << std::endl;

    std::cout << "   ";
    for (const auto& c1 : rv1)
    {
        std::cout << c1;
    }
    std::cout << std::endl;

    ii = 0;
    for (const auto& c2 : rv2)
    {
        char cc = '0' + (ii % 10);
        if (cc == '0') cc = '.';
        std::cout << cc;

        std::cout << c2 << " ";
        for (const auto& c1 : rv1)
        {
            std::cout << ((c1 == c2) ? '@' : '+');
        }
        std::cout << std::endl;
        ii++;
    }
    std::cout << std::endl;
}


void dump_match(
    const size_t maxlen,
    const sequtil::T_PT& rpt,
    const std::vector<char>& rv1,
    const std::vector<char>& rv2)
{
    std::cout << rpt.row << "," << rpt.col;
    std::cout << "  ";
    for (size_t ii = rpt.row + 1 - maxlen; ii <= rpt.row; ii++)
    {
        std::cout << rv2[ii];
    }
    std::cout << " ";
    for (size_t ii = rpt.col + 1 - maxlen; ii <= rpt.col; ii++)
    {
        std::cout << rv1[ii];
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

    // fill the shortest vector first
    if (n1 < n2)
    {
        for (size_t ii = 0; ii < n1; ii++) v1[ii] = 'A' + (rand() % sym_ct);
        for (size_t ii = 0; ii < n2; ii++) v2[ii] = 'A' + (rand() % sym_ct);
    }
    else
    {
        for (size_t ii = 0; ii < n2; ii++) v2[ii] = 'A' + (rand() % sym_ct);
        for (size_t ii = 0; ii < n1; ii++) v1[ii] = 'A' + (rand() % sym_ct);
    }

    if (is_dump)
    {
        dump_grid(v1, v2);
    }

    WinExTmr extmr;
    sequtil::T_MAP_SZ2PT result;
    extmr.start();
    seqx.find_max(v1, v2, result);
    extmr.stop();
    std::cout << extmr.elapsed_time() << std::endl;

    std::cout << seqx.index_load_factor << std::endl;
    std::cout << seqx.index_max_load_factor << std::endl;
    std::cout << seqx.index_bucket_count << std::endl;
    std::cout << "Max Map Sz = " << seqx.max_map_size << std::endl;
    const auto& qlast = result.crbegin();
    size_t maxlen = qlast->first;
    std::cout << "Max Length = " << maxlen << std::endl;
    for (const auto qpt : qlast->second)
    {
        dump_match(maxlen, qpt, v1, v2);
    }
}


void test_str_max(
    const std::string& rs1,
    const std::string& rs2,
    const bool is_dump = false)
{
    std::cout << "-------------------" << std::endl;

    std::cout << "[" << rs1 << "], [" << rs2 << "]" << std::endl;

    std::vector<char> v1(rs1.size());
    std::vector<char> v2(rs2.size());
    std::copy(rs1.begin(), rs1.end(), v1.begin());
    std::copy(rs2.begin(), rs2.end(), v2.begin());

    if (is_dump)
    {
        dump_grid(v1, v2);
    }

    sequtil::SeqMatch<char> seqx;
    sequtil::T_MAP_SZ2PT result;
    seqx.find_max(v1, v2, result);
    
    if (result.empty())
    {
        std::cout << "empty" << std::endl;
    }
    else
    {
        const auto& qlast = result.crbegin();
        size_t maxlen = qlast->first;
        std::cout << "Max Length = " << maxlen << std::endl;
        for (const auto qpt : qlast->second)
        {
            dump_match(maxlen, qpt, v1, v2);
        }
    }
}


void test_str_combos(void)
{
    test_str_max("BBCCAAAAABBAAAA", "AAAAACCBBBBAAA", true);

    test_str_max("", "ABCD");
    test_str_max("ABCD", "");
    test_str_max("WXYZ", "ABCDEFG");
    test_str_max("ABCDEFG", "WXYZ");
    
    test_str_max("ABC", "ABCDEFG");
    test_str_max("CDE", "ABCDEFG");
    test_str_max("EFG", "ABCDEFG");
    test_str_max("ABCDEFG", "ABC");
    test_str_max("ABCDEFG", "CDE");
    test_str_max("ABCDEFG", "EFG");
    
    test_str_max("ABCDEFG", "ABCDEFG");
    test_str_max("AAAAAAA", "AAAAAAA");

    test_str_max("AAA", "AAAAAAA");
    test_str_max("AAAAAAA", "AAA");
}


void test_ran_sym(const int sym_ct)
{
    test_random_char(12345, 2, 100, 24, true);

    // smallest vector should be first
    test_random_char(12345, sym_ct, 1000, 1000);
    test_random_char(12345, sym_ct, 1000, 10000);
    test_random_char(12345, sym_ct, 1000, 100000);
    test_random_char(12345, sym_ct, 1000, 1000000);
}


int main(int argc, char * argv[])
{
    std::cout << "Longest Common Sequence Finder\n";

    int sym_ct = 20;
    int nsquare = 10;
    bool is_square = false;

    if (argc >= 2)
    {
        sym_ct = atoi(argv[1]);
        sym_ct = std::max<int>(2, sym_ct);
        sym_ct = std::min<int>(sym_ct, 100);
    }

    if (argc >= 3)
    {
        is_square = true;
        nsquare = atoi(argv[2]);
        nsquare = std::max<int>(10, nsquare);
        nsquare = std::min<int>(nsquare, 1000000);
    }

    if (is_square)
    {
        std::cout << "Random " << nsquare << "x" << nsquare;
        std::cout << " with " << sym_ct << " symbols" << std::endl;
        test_random_char(12345, sym_ct, nsquare, nsquare, (nsquare <= 40));
    }
    else
    {
        test_str_combos();
        test_ran_sym(sym_ct);
    }

    return 0;
}
