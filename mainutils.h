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
