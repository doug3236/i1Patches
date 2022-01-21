#pragma once
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <optional>
#include <utility>
#include <thread>
#include <chrono>
#include "cgats.h"
#include "cgats2.h"
#include "statistics.h"
#include "deltae.h"

using std::array;
using std::vector;
using std::string;
using std::pair;
using std::cout;
using std::endl;
using std::tuple;

using V3 = array<float, 3>;
using V6 = array<float, 6>;
using RGB = V3;

using namespace cgats_utilities;

pair<vector<V3>,vector<float>> refine_lab(const vector<V3>& rgb, string prof, bool rev_only=false);



inline RGB operator-(const RGB& a, const RGB& b) { return RGB{ a[0] - b[0], a[1] - b[1], a[2] - b[2] }; }
inline RGB operator+(const RGB& a, const RGB& b) { return RGB{ a[0] + b[0], a[1] + b[1], a[2] + b[2] }; }

class CheckColors {
public:
    struct PatchValues {
        vector<V3> labout;
        vector<V3> labhigh;
        vector<V3> lablow;
    };
    string filename;
    CheckColors(string filename, vector<V6> rgblab, PatchHeader header);
    //CheckColors(string filename, vector<V6> rgblab);
    void print(string filename);
    vector<V3> get_slope();
    static constexpr int N = 10;
    const char* color_descr(int color) const;
    vector<V3> rgb;						// all patches, rgb
    vector<V3> lab;						// all patches, lab
    vector<vector<V3>> page_rgb_raw;
    vector<vector<V3>> page_lab_raw;
    int pages;
    int colors_per_page;
    int colors10;
    vector<V3> colors_rgb;
    int patches_per_page;

    vector<vector<V3>> page_labs_all;	// all ref colors on each page

    vector<V3> labs_mean;				        // mean all ref colors
    vector<vector<V3>> page_labs_mean;	        // mean all ref colors on each page
    vector<vector<V3>> page_labs_std;	        // standard dev on each page
    array<StatV3,10> color_stats; // stats on each color

    vector<double> std_page;			// mean std of ref colors per page
    // relative to all page mean
    vector<vector<V3>> page_lab_delta_high;
    vector<vector<V3>> page_lab_delta_low;
    vector<vector<std::pair<double, double>>> page_color_maxmin;
};

void print2(const CheckColors& file1, const CheckColors& file2);
void print_fwd_rev_stats(CGATS_Bidir& cgats, CheckColors *ref_colors=nullptr);
void print_dE_distr(vector<float> dE1, vector<float> dE2 = vector<float>());

PatchSet gather_patch_info(const vector<V6>& rgblab);


struct RGB_LABM_LABI {
    int index{};
    V3 rgb{};
    V3 labm{};
    V3 labi{};
    float dE76{};
    float dE2000{};
    float rgberr;
};

struct PatchCollection {
    vector<RGB_LABM_LABI> full;
    vector<RGB_LABM_LABI> ave;
};

inline vector<V3> get_rgb(const vector<RGB_LABM_LABI>& v)
{
    vector<V3> ret(v.size());
    for (size_t i = 0; i < v.size(); i++)
        ret[i] = v[i].rgb;
    return ret;
}
inline vector<V3> get_labm(const vector<RGB_LABM_LABI>& v)
{
    vector<V3> ret(v.size());
    for (size_t i = 0; i < v.size(); i++)
        ret[i] = v[i].labm;
    return ret;
}
inline vector<V3> get_labi(const vector<RGB_LABM_LABI>& v)
{
    vector<V3> ret(v.size());
    for (size_t i = 0; i < v.size(); i++)
        ret[i] = v[i].labi;
    return ret;
}

// -C requires same duplication algo as in -T
inline PatchCollection make_rgb_labm_labi(const vector<V6>& rgblab)
{
    auto [dup, pcount] = gather_patch_info(rgblab);
    vector<RGB_LABM_LABI> v(pcount * dup);
    vector<RGB_LABM_LABI> v_ave(pcount);
    for (size_t i = 0; i < pcount; i++)
    {
        V3 labm{};
        for (size_t ii = 0; ii < dup; ii++)
        {
            v[i * dup + ii].rgb = *(V3*)(&rgblab[i * dup + ii]);
            labm = labm + (v[i * dup + ii].labm = *((V3*)(&rgblab[i * dup + ii]) + 1));
            v[i * dup + ii].index = int(i * dup + ii + 1);
        }
        // Now update averaged RGB_LABM_LABI value
        for (auto& x : labm)  x /= dup; // compute average
        v_ave[i].labm = labm;
        v_ave[i].index = int(i + 1);
        v_ave[i].rgb = v[i * dup].rgb;
    }
    return { v, v_ave };
}

// refine labs that produces RGB values then calculate dEs from the measured Labs
inline bool update_labi_from_profile(PatchCollection& v, const string& profile_image, bool rev_only)
{
    size_t pcount = v.ave.size();
    size_t dup = v.full.size() / pcount;
    vector<V3> rgb = get_rgb(v.ave);
    // if !rev_only, searches for Lab that produces RGB values. Failure returns 99 in labi.second
    // failure substitutes reverse lookup. s/b less than 10% of cases
    pair<vector<V3>, vector<float>> labi = refine_lab(rgb, profile_image, rev_only);
    for (size_t i = 0; i < rgb.size(); i++)
    {
        v.ave[i].labi = labi.first[i];
        v.ave[i].dE76 = deltaE(v.ave[i].labm, v.ave[i].labi, 0);
        v.ave[i].dE2000 = deltaE(v.ave[i].labm, v.ave[i].labi, 1);
        v.ave[i].rgberr = labi.second[i];
    }
    for (size_t i = 0; i < pcount; i++)
    {
        for (size_t ii = 0; ii < dup; ii++)
        {
            v.full[i * dup + ii].labi = v.ave[i].labi;  // these are generated from rgb values
            v.full[i * dup + ii].dE76 = deltaE(v.full[i * dup + ii].labm, v.ave[i].labi, 0);
            v.full[i * dup + ii].dE2000 = deltaE(v.full[i * dup + ii].labm, v.ave[i].labi, 1);
            v.full[i * dup + ii].rgberr = v.ave[i].rgberr;
        }
    }
    return true;
}

inline vector<RGB_LABM_LABI> get_rgb_labm_labi_averaged(const vector<RGB_LABM_LABI>& v, int dup)
{
    if (dup == 1)
        return v;
    vector<RGB_LABM_LABI> ret(v.size() / dup);
    for (size_t i = 0; i < v.size(); i += dup)
    {
        ret[i / dup] = v[i];
        for (size_t ii = 1; ii < dup; ii++)
            ret[i / dup].labm = ret[i / dup].labm + v[i+ii].labm;
    }
    for (auto& x : ret)
        for (auto& xx : x.labm)
            xx /= dup;
    return ret;
}

