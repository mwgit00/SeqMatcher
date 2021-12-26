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

    struct T_PT_Compare
    {
        bool operator() (const T_PT& a, const T_PT& b) const
        {
            if (a.row == b.row) return a.col < b.col;
            return a.row < b.row;
        }
    };

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

        void find_all(
            const std::vector<T>& rv1,
            const std::vector<T>& rv2,
            T_MAP_SZ2PT& rmatches);

        void find_max(
            const std::vector<T>& rv1,
            const std::vector<T>& rv2,
            T_MAP_SZ2PT& rmatches);
    public:
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
    }


    template <class T>
    SeqMatch<T>::SeqMatch() :
        max_map_size(0)
    {

    }


    template <class T>
    SeqMatch<T>::~SeqMatch()
    {

    }


    template <class T>
    void SeqMatch<T>::find_all(
        const std::vector<T>& rv1,
        const std::vector<T>& rv2,
        T_MAP_SZ2PT& rmatches)
    {
        std::unordered_map<T, T_VEC_SZ> map_sym1_to_pos;
        std::map<T_PT, size_t, T_PT_Compare> map_pt_to_seqlen;

        // solution is empty if either vector is empty
        rmatches.clear();
        if ((rv1.size() == 0) || (rv2.size() == 0))
        {
            return;
        }

        // rv1 is the "horizontal" (column index) data
        // create lookup table that maps each symbol
        // to all the locations where it occurs

        build_index(rv1, map_sym1_to_pos);

        // rv2 is the "vertical" (row index) data
        // traverse it and match each symbol against rv1

        for (size_t jj = 0; jj < rv2.size(); jj++)
        {
            const T_VEC_SZ& rpos = map_sym1_to_pos[rv2[jj]];
            for (const auto& ix : rpos)
            {
                size_t linklen = 0;
                if ((jj > 0) && (ix > 0))
                {
                    // check if this match is "diagonal link"
                    // to an existing (old) sequence
                    T_PT pt_old;
                    pt_old.row = static_cast<uint32_t>(jj) - 1;
                    pt_old.col = static_cast<uint32_t>(ix) - 1;
                    if (map_pt_to_seqlen.count(pt_old))
                    {
                        linklen = map_pt_to_seqlen[pt_old];
                        map_pt_to_seqlen.erase(pt_old);
                    }
                }
                T_PT pt_new;
                pt_new.row = static_cast<uint32_t>(jj);
                pt_new.col = static_cast<uint32_t>(ix);
                map_pt_to_seqlen.insert({ pt_new, 1 + linklen });
            }
        }

        // remap sequence lengths to
        // locations where sequences of that length occur
        rmatches.clear();
        for (const auto& r : map_pt_to_seqlen)
        {
            if (rmatches.count(r.second) == 0)
            {
                rmatches.insert({ r.second, {} });
            }
            rmatches[r.second].push_back(r.first);
        }

        // clean up
        max_map_size = map_pt_to_seqlen.size();
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
                // to an existing sequence in map
                // if jj or ix is 0 the key will be bogus but that's okay
                size_t linklen = 0;
                T_PTX ptx;
                ptx.pt.row = static_cast<uint32_t>(jj) - 1;
                ptx.pt.col = static_cast<uint32_t>(ix) - 1;
                uint64_t key = ptx.rowcol;
                if (map_pt_to_seqlen.count(key))
                {
                    // sequence can be extended
                    // the old map entry can be removed
                    linklen = map_pt_to_seqlen[key];
                    map_pt_to_seqlen.erase(key);
                }

                // insert new link in sequence
                // and update new max sequence length
                size_t newlinklen = 1 + linklen;
                ptx.pt.row++;
                ptx.pt.col++;
                map_pt_to_seqlen.insert({ ptx.rowcol, newlinklen });
                max_len = std::max<size_t>(max_len, newlinklen);
            }

            max_map_size = std::max<size_t>(max_map_size, map_pt_to_seqlen.size());

            // remove dead ends
            // from previous rows
            for (auto iter = map_pt_to_seqlen.begin(); iter != map_pt_to_seqlen.end(); )
            {
                T_PTX ptx_key;
                ptx_key.rowcol = iter->first;
                if ((iter->second < max_len) && (jj > ptx_key.pt.row))
                {
                    ptx_key.pt.row++;
                    ptx_key.pt.col++;
                    if (map_pt_to_seqlen.count(ptx_key.rowcol) == 0)
                    {
                        // this point can't be part of a sequence
                        // longer than the current max so delete it
                        iter = map_pt_to_seqlen.erase(iter);
                    }
                    else
                    {
                        iter++;
                    }
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
    }
}

#endif // SEQ_MATCH_H_
