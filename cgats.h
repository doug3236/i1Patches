/*
Copyright (c) <2019> <doug gray>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <vector>
#include <utility>
#include <string>
#include <array>
#include <fstream>
#include <iostream>
#include <tuple>

#include "statistics.h"
#include "validate.h"

using std::vector;
using std::string;
using std::pair;
using std::array;

namespace cgats_utilities {
    // gathers CGATs file info
    struct Cgats_data {
        string filename;
        vector<vector<string>> lines;   // tokenized lines in CGATs file
        vector<string> fields;          // tokenized list of fields
        vector<vector<string>>::iterator data_ptr; // pointer to start of value lines
        int num_of_fields;  // num_of_fields==fields.size()==data_ptr[0].size()
        int num_of_sets;    // data_ptr[0] to data_ptr[num_of_sets-1]
        ptrdiff_t rgb_loc;  // location of RGB_R in fields
        ptrdiff_t lab_loc;  // location of LAB_L in fields
    };
    Cgats_data populate_cgats(const string& filename);

    using V3=array<float, 3>;  // typically used to hold RGB, LAB or XYZ values
    using V6=array<float, 6>;  // typically used to hold RGBLAB value sets

    vector<V6> read_cgats_rgblab(const string& filename, bool include_lab = true);
    vector<V3> read_cgats_rgb(const string& filename);

    bool write_cgats_rgb(const vector<V3>& rgb, const string& filename);  // write rgb patch file
    bool write_cgats_rgblab(const vector<V6>& rgblab, const string& filename, string descriptor = "RGBLAB");
    pair<vector<V3>, vector<V3>> separate_rgb_lab(const vector<V6>& rgblab);
    vector<V6> combine_rgb_lab(const vector<V3>& rgb, const vector<V3>& lab);

    // predicate for sort by RGB values
    bool less_than(const V6& arg1, const V6& arg2);
    // Overrides operator== for array<> to look at only the first 3 values
    bool operator==(const V6& arg1, const V6& arg2);
    
    inline string remove_suffix(std::string fname)
    {
        size_t dot_loc = fname.find_last_of('.');
        return fname.substr(0, dot_loc);
    }

    // Usage: replace_suffix("test.icm", ".icm", "_adj.icm");
    inline std::string replace_suffix(std::string name, string suffix, std::string replacement)
    {
        if (name.size() <= suffix.size())
            throw std::invalid_argument("Suffix longer than name.");
        if (name.substr(name.size() - suffix.size(), suffix.size()) != suffix)
            throw std::invalid_argument("suffix doesn't match.");
        string sname = name.substr(0, name.size() - suffix.size());
        return sname + replacement;
    }
}

