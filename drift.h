#pragma once
#include <vector>
#include <array>
#include <string>
#include <random>
#include <numeric>
#include <unordered_map>
#include <functional>
#include "cgats2.h"

using std::vector;
using std::array;
using std::string;
using std::pair;
using V3 = array<float, 3>;
using V6 = array<float, 6>;

struct HashV3
{
    std::size_t operator()(V3 const& f) const noexcept
    {
        return std::hash<float>{}(f[0]) ^ std::hash<float>{}(f[1]) ^ std::hash<float>{}(f[2]);
    }
};

struct PatchData {
    vector<V3> rgb;     // RGB patches. Either neutrals or color grid 4x4x4
    vector<V3> lab;     // average of all same patches
    vector<float> dE;   // dE average of same color patches against mean
    vector<float> dE2k; // dE2k average of same color patches against mean
    float dE_ave;       // Overall dE average
    float dE2k_ave;
};

struct PatchGroup {
    vector<V3> rgb;
    vector<V3> lab;
    vector<size_t> count;
    vector<float> dE;
    vector<float> dE2k;
};

class ProcessDuplicates {
public:
    ProcessDuplicates();
    ProcessDuplicates(vector<V6> in_rgblab);
    vector<V3> get_page_rgb(size_t patch_cnt) {
        vector<V3> rgb_page;       // full page
        for (size_t i = 0; i < patch_cnt; i++)
            rgb_page.push_back(defined_rgb[i % defined_rgb.size()]);
        return randomize(rgb_page); // randomize with seed=1
    }
    vector<V6> get_both_averaged_rgblab() const;
private:
    vector<V6> rgblab;              // copy of input rgb:lab
    bool defined_rgb_valid = true;  // if standard drift format. 60 colors, 19 neutrals
    vector<V3> defined_rgb;         // drift format: 19 neutrals then 60 colors
    PatchGroup neutrals_p;
    PatchGroup colors_p;
    PatchGroup both_p;
    friend void print(const ProcessDuplicates& first, const ProcessDuplicates& second);
};

void print(const ProcessDuplicates& first, const ProcessDuplicates& second= ProcessDuplicates());

