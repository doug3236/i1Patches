#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <array>
#include <string>
#include <random>
#include <algorithm>
#include "cgats.h"
#include "cgats2.h"
#include "statistics.h"

using std::array;
using std::vector;
using std::string;
using RGB = array<float, 3>;


struct RGB2_dE
{
    RGB rgb1, rgb2;
    float dE;
};

struct Summary
{
    vector<RGB> unique_rgb;
    vector<RGB2_dE> rgb2de;
    float dist_ave{};
    float dist_min{};
    float dist_max{};
    float dist_max_min{};
    float dist_std{};
};

struct Distr_RGB
{
    size_t nend{};          // final number of randomly but spread, RGB triplets
    size_t ntotal{};        // total number of RGB triplets before removal of close points
    size_t seed{};          // randomizer seed, 0 each run differs, <0 :use this seed
    vector<RGB> rgb_list;   // all RGB triplets. indexed lookup by idx... for speed

    string profile_image{}; // optional image of printer profile
    vector<RGB> lab_list;   // L*a*b* value of printer's RGB values if profile_image

    float threshold{};      // threshold for removal of distant point pairs
    struct IDX_De           // point pairs and dE between them
    {
        uint16_t idx1;
        uint16_t idx2;
        float dE;
    };
    vector<IDX_De> idx_de;  // indexes of all point pairs and dE between them
    vector<RGB> unique_rgb;
    vector<RGB2_dE> rgb2de;

    Summary make_random_separated_rgb(size_t nstart, size_t nend, unsigned int seed);

private:
    void initialize_rgb(size_t count, unsigned int seed);
};

