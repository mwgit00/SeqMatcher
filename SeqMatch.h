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

#ifndef SEQ_MATCH_H_
#define SEQ_MATCH_H_

#include <map>
#include <vector>
#include <unordered_map>

namespace sequtil
{
#pragma pack(push, 1)
    typedef struct
    {
        uint32_t row;
        uint32_t col;
    } T_PT;
#pragma pack(pop)

#pragma pack(push, 1)
    typedef union
    {
        T_PT pt;            // row and col accessible by name
        uint64_t rowcol;    // combines row and col into one 64-bit value
    } T_PTX;
#pragma pack(pop)

    typedef std::vector<T_PT> T_VEC_PT;
    typedef std::vector<size_t> T_VEC_SZ;
    typedef std::map<size_t, T_VEC_PT> T_MAP_SZ2PT;

    template <class T>
    class SeqMatch
    {
    public:
        SeqMatch();
        virtual ~SeqMatch();

        void build_index(
            const std::vector<T>& rv1,
            std::unordered_map<T, T_VEC_SZ>& rmap);

        void find_max(
            const std::vector<T>& rv1,
            const std::vector<T>& rv2,
            T_MAP_SZ2PT& rmatches);

        void find_max3(
            const std::vector<T>& rv1,
            const std::vector<T>& rv2,
            T_MAP_SZ2PT& rmatches);

    public:
        float index_load_factor;
        float index_max_load_factor;
        size_t index_bucket_count;
        size_t max_map_size;
    };


    template <class T>
    void SeqMatch<T>::build_index(
        const std::vector<T>& rv,
        std::unordered_map<T, T_VEC_SZ>& rmap)
    {
        rmap.clear();
        for (size_t ii = 0; ii < rv.size(); ii++)
        {
            // if key not found yet then insert it
            // and add index to list
            if (rmap.count(rv[ii]) == 0)
            {
                rmap.insert({ rv[ii], {} });
            }
            rmap[rv[ii]].push_back(ii);
        }
#if 0
        index_load_factor = rmap.load_factor();
        index_max_load_factor = rmap.max_load_factor();
        index_bucket_count = rmap.bucket_count();
#endif
    }


    template <class T>
    SeqMatch<T>::SeqMatch() :
        index_load_factor(0.0),
        index_max_load_factor(0.0),
        index_bucket_count(0),
        max_map_size(0)
    {

    }


    template <class T>
    SeqMatch<T>::~SeqMatch()
    {

    }


    template <class T>
    void SeqMatch<T>::find_max(
        const std::vector<T>& rv1,
        const std::vector<T>& rv2,
        T_MAP_SZ2PT& rmatches)
    {
        rmatches.clear();
        max_map_size = 0;

        std::unordered_map<T, T_VEC_SZ> map_sym1_to_pos;
        std::unordered_map<uint64_t, size_t> map_pt_to_seqlen;

        if ((rv1.size() == 0) || (rv2.size() == 0))
        {
            // solution is empty if either vector is empty
            return;
        }

        // rv1 is the "horizontal" (column index) data
        // create lookup table that maps each symbol
        // to all the locations where it occurs

        build_index(rv1, map_sym1_to_pos);

        // rv2 is the "vertical" (row index) data
        // traverse it and match each symbol against rv1

        size_t max_len = 1;
        for (size_t jj = 0; jj < rv2.size(); jj++)
        {
            const T_VEC_SZ& rpos = map_sym1_to_pos[rv2[jj]];
            for (const auto ix : rpos)
            {
                // check if this match is "diagonal link"
                // to an existing sequence record in map
                // if jj or ix is 0 the key will be bogus but that's okay
                size_t linklen = 0;
                T_PTX ptx;
                ptx.pt.row = static_cast<uint32_t>(jj) - 1;
                ptx.pt.col = static_cast<uint32_t>(ix) - 1;
                auto iter = map_pt_to_seqlen.find(ptx.rowcol);
                if (iter != map_pt_to_seqlen.end())
                {
                    // sequence can be extended
                    // the old map entry can be removed
                    linklen = iter->second;
                    map_pt_to_seqlen.erase(iter);
                }

                // insert new link in sequence
                // and update new max sequence length
                size_t newlinklen = 1 + linklen;
                ptx.pt.row++;
                ptx.pt.col++;
                map_pt_to_seqlen.emplace(std::make_pair(ptx.rowcol, newlinklen));
                max_len = std::max<size_t>(max_len, newlinklen);
            }

            max_map_size = std::max<size_t>(max_map_size, map_pt_to_seqlen.size());

            // remove dead ends from previous rows
            for (auto iter = map_pt_to_seqlen.begin(); iter != map_pt_to_seqlen.end(); )
            {
                T_PTX ptx_key;
                ptx_key.rowcol = iter->first;
                if ((ptx_key.pt.row < jj) && (iter->second < max_len))
                {
                    iter = map_pt_to_seqlen.erase(iter);
                }
                else
                {
                    iter++;
                }
            }
        }

        // collect the data for the longest sequence
        // there may be multiple sequences of that same length
        if (map_pt_to_seqlen.size() > 0)
        {
            rmatches.insert({ max_len, {} });
            for (const auto& r : map_pt_to_seqlen)
            {
                if (r.second == max_len)
                {
                    T_PTX ptx;
                    ptx.rowcol = r.first;
                    rmatches[max_len].push_back(ptx.pt);
                }
            }
        }

#if 1
        index_load_factor = map_pt_to_seqlen.load_factor();
        index_max_load_factor = map_pt_to_seqlen.max_load_factor();
        index_bucket_count = map_pt_to_seqlen.bucket_count();
#endif
    }


    template <class T>
    void SeqMatch<T>::find_max3(
        const std::vector<T>& rv1,
        const std::vector<T>& rv2,
        T_MAP_SZ2PT& rmatches)
    {
        rmatches.clear();
        max_map_size = 0;

        std::map<T, T_VEC_SZ> map_sym1_to_pos;
        std::unordered_map<size_t, std::unordered_map<size_t, size_t>> map_work;

        if ((rv1.size() == 0) || (rv2.size() == 0))
        {
            // solution is empty if either vector is empty
            return;
        }

        // rv1 is the "horizontal" (column index) data
        // create lookup table that maps each symbol
        // to all the locations where it occurs

        build_index(rv1, map_sym1_to_pos);

        // rv2 is the "vertical" (row index) data
        // traverse it and match each symbol against rv1

        size_t max_len = 1;
        for (size_t jj = 0; jj < rv2.size(); jj++)
        {
            const T_VEC_SZ& rpos = map_sym1_to_pos[rv2[jj]];
            for (const auto ix : rpos)
            {
                // check if this match is "diagonal link"
                // to an existing sequence record in map
                // if jj or ix is 0 the key will be bogus but that's okay
                size_t linklen = 0;
                size_t row = static_cast<uint32_t>(jj) - 1;
                size_t col = static_cast<uint32_t>(ix) - 1;
                auto iter_row = map_work.find(row);
                if (iter_row != map_work.end())
                {
                    auto& rmap = iter_row->second;
                    auto iter_col = rmap.find(col);
                    if (iter_col != rmap.end())
                    {
                        // sequence can be extended
                        // the old map entry can be removed
                        linklen = iter_col->second;
                        rmap.erase(iter_col);
                        if (rmap.size() == 0)
                        {
                            map_work.erase(iter_row);
                        }
                    }
                }

                // insert new link in sequence
                // and update new max sequence length
                size_t newlinklen = 1 + linklen;
                map_work[jj][ix] = newlinklen;
                max_len = std::max<size_t>(max_len, newlinklen);
            }

            // remove dead ends from previous rows
            for (auto iter_row = map_work.begin(); iter_row != map_work.end(); )
            {
                if (iter_row->first == jj)
                {
                    break;
                }

                auto& rmap = iter_row->second;
                for (auto iter_col = rmap.begin(); iter_col != rmap.end(); )
                {
                    if (iter_col->second < max_len)
                    {
                        iter_col = rmap.erase(iter_col);
                    }
                    else
                    {
                        iter_col++;
                    }
                }

                if (rmap.size() == 0)
                {
                    iter_row = map_work.erase(iter_row);
                }
                else
                {
                    iter_row++;
                }
            }
        }

        // collect the data for the longest sequence
        // there may be multiple sequences of that same length
        if (map_work.size() > 0)
        {
            rmatches.insert({ max_len, {} });
            for (const auto& rr : map_work)
            {
                for (const auto& rc : rr.second)
                {
                    if (rc.second == max_len)
                    {
                        T_PT pt;
                        pt.row = static_cast<uint32_t>(rr.first);
                        pt.col = static_cast<uint32_t>(rc.first);
                        rmatches[max_len].push_back(pt);
                    }
                }
            }
        }
    }
}

#endif // SEQ_MATCH_H_
