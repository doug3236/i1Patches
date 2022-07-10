#pragma once

#include <vector>
#include <array>
#include <string>
#include <tuple>
#include <algorithm>
#include <numeric>


#include "statistics.h"
#include "checkcolors.h"

using std::vector;
using std::array;
using std::string;
using V3 = array<float, 3>;

struct Lab_dE_Set {
    vector<float> dE76;     // sorted for histogram
    vector<float> dE2000;
    float mean_dE_76;       // mean dE
    float mean_dE_2000;
    float mean_std_dE_76;   // standard deviation of the mean
    float mean_std_dE_2000;
    float mean_saturation;  // mean saturation
};

vector<Lab_dE_Set> get_color_info(vector<V3> laba, vector<V3> labb, string profname, int neutral_size);

// Neutral tests sequential L* increases
// Check seq for 52 or 18 neutrals or close neutrals and validate end point.
int get_size_of_neutrals(PatchCollection& vc);

struct ColorAccuracy {
    string profile_name;
    float dE76_dev_neutrals{};
    float dE76_low_saturation{};
    float dE76_full_gamut{};
    float dE2k_dev_neutrals{};
    float dE2k_low_saturation{};
    float dE2k_full_gamut{};
};

ColorAccuracy print_dE_stats(PatchCollection& collection, string prof_name, bool col_ave);
vector<std::pair<int, int>> get_optional_range_v(size_t cgats_size, int& argc, char**& argv);


// global traits for ACPU and/or FRACTIONAL options
// Must precede any other arguments/options
// ACPU increases tiff dimensions 2% to comepsate ACPU utility shrinkage
// FRACTIONAL allows fractional RGB values and dithers 8 bit tiff charts
struct global_modifiers {
    inline static bool expand_tif_2_percent;
    inline static bool fractional;
    inline static std::vector<std::pair<string, bool&>> options{ {"ACPU", expand_tif_2_percent },{"FRACTIONAL", fractional} };
    inline static void update(int& argc, char**& argv)
    {
        for (auto& x : options)
        {
            if (argc < 2)
                return;
            if (x.first == argv[1])
            {
                x.second = true;
                argv++; argc--;
            }
        }
    }
};

// Conversion to/from isis/i1pro2 structure
// i1pro2 reads in multiple pages by concatinating rows for 1 virtual page with rows=21*num_of_pages
// i1isis organizes in page size blocks. This function converts to/from i1isis and i1pro2 
template<typename T>
vector<T> change_i1_order(const vector<T>& v, int row_count, int col_count, bool to_i1Pro2)
{
    vector<T> ret(v.size());
    int page_count = (int)v.size() / (row_count * col_count);
    for (int i = 0; i < (int)v.size(); i++)
        if (to_i1Pro2)
        {
            int o_page = i / (row_count * col_count);       // physical page
            int o_page_offset = i % (row_count * col_count);
            int o_row = o_page_offset % row_count;
            int o_col = o_page_offset / row_count;
            int loc = o_row + o_page * row_count + o_col * page_count * row_count;
            ret[loc] = v[i];
        }
        else
        {
            int row = i % (row_count * page_count); // row in i1Pro2 virtual single page equiv
            int col = i / (row_count * page_count); // column in i1Pro2 virtual single page equiv
            int page = row / row_count;             // physical page virtual row is in
            int row_page = row % row_count;         // row within physical page
            int loc = page * row_count * col_count + row_page + col * row_count;    // location in i1isis format
            ret[loc] = v[i];
        }
    return ret;
}


// debugging test
inline void test_change()
{
    vector<int> tmp(4 * 5 * 2);
    std::iota(tmp.begin(), tmp.end(), 0);    // s/b 0 to 39
    tmp = change_i1_order(tmp, 4, 5, true);
    tmp = change_i1_order(tmp, 4, 5, false);    // s/b 0 to 39
}
