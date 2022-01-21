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
// Disable errors for fread and fwrite posix functions
#define _CRT_SECURE_NO_WARNINGS

#include "cgats.h"
#include <fstream>
#include <utility>
#include <sstream>
#include <iomanip>
#include <algorithm>
#pragma warning(disable : 4996)

namespace cgats_utilities
{
    // tokenize a line and return vector of tokens, token may be quoted
    static vector<string> parse(const string& s) {
        std::stringstream ss(s);
        vector<string> ret;
        for (;;) {
            string tmp;
            ss >> std::quoted(tmp);
            if (tmp.size() == 0)
                break;
            ret.push_back(tmp);
        }
        return ret;
    };

    // Parse file returning a vector<vector<string>> tokens. vector<string> tokenizes one line
    static vector<vector<string>> tokenize_file(const string& filename)
    {
        std::ifstream infile(filename);
        if (infile.fail())
            throw std::invalid_argument(string("Read Open failed, File: ") + filename);
        string s;
        vector<vector<string>> ret;
        while (getline(infile, s))
        {
            vector<string> line_words = parse(s);
            if (line_words.size() != 0)             // skip empty lines
                ret.push_back(line_words);
        }
        return ret;
    }

    // Parse and get CGATs structure
    Cgats_data populate_cgats(const string& filename)
    {
        Cgats_data data;
        data.filename = filename;
        data.lines = tokenize_file(filename);
        data.num_of_fields = stoi(std::find_if(data.lines.begin(), data.lines.end(),
            [](const vector<string>& arg) {return arg[0] == "NUMBER_OF_FIELDS"; })[0][1]);
        data.num_of_sets = stoi(std::find_if(data.lines.begin(), data.lines.end(),
            [](const vector<string>& arg) {return arg[0] == "NUMBER_OF_SETS"; })[0][1]);

        data.fields = std::find_if(data.lines.begin(), data.lines.end(),
            [](const vector<string>& arg) {return arg[0] == "BEGIN_DATA_FORMAT"; })[1];
        validate(data.num_of_fields == data.fields.size(), "Bad CGATS Format, num of fields <> field count");
        data.data_ptr = std::find_if(data.lines.begin(), data.lines.end(),
            [](const vector<string>& arg) {return arg[0] == "BEGIN_DATA"; }) + 1;

        // Note: if RGB_R or LAB_L don't exist returns data.fields.size()
        data.rgb_loc = std::find_if(data.fields.begin(), data.fields.end(),
            [](const string& arg) {return arg == "RGB_R"; }) - data.fields.begin();
        data.lab_loc = std::find_if(data.fields.begin(), data.fields.end(),
            [](const string& arg) {return arg == "LAB_L"; }) - data.fields.begin();
        return data;
    }


    // Read CGATs file RGB values and optional LAB values.
    // Structure is read but not used for easy extension.
    vector<V6> read_cgats_rgblab(const string& filename, bool include_lab)
    {
        Cgats_data data = populate_cgats(filename);
        vector<V6> rgb_lab(data.num_of_sets);
        for (size_t i = 0; i < rgb_lab.size(); ++i)
        {
            for (size_t ii = 0; ii < 3; ++ii)
            {
                validate(data.data_ptr[i].size() == data.fields.size(), "Bad CGATS Format, num of vals <> field count");
                rgb_lab[i][ii] = stof(data.data_ptr[i][data.rgb_loc + ii]);
                if (include_lab && data.num_of_fields != data.lab_loc)
                    rgb_lab[i][ii+3] = stof(data.data_ptr[i][data.lab_loc + ii]);
            }
        }
        return rgb_lab;
    }

    // This reads only the RGB components of a CGATs file
    vector<V3> read_cgats_rgb(const string& filename)
    {
        vector<V6> rgblab = read_cgats_rgblab(filename, false);
        vector<V3> ret;
        for (auto x : rgblab)
            ret.emplace_back(V3{ x[0], x[1], x[2] });
        return ret;
    }

    // Writes minimalist RGB CGATs file
    bool write_cgats_rgb(const vector<V3>& rgb, const string& filename)
    {
        FILE* fp = fopen(filename.c_str(), "wt");
        if (!fp)
            throw std::invalid_argument("File Open for Write Failed.");
        fprintf(fp, "NUMBER_OF_FIELDS 4\n"
            "BEGIN_DATA_FORMAT\n"
            "SampleID	RGB_R	RGB_G	RGB_B\n"
            "END_DATA_FORMAT\n\n"
            "NUMBER_OF_SETS	%d\n"
            "BEGIN_DATA\n", static_cast<int>(rgb.size()));
        for (unsigned int i = 0; i < rgb.size(); i++)
            fprintf(fp, "%d\t%6.2f\t%6.2f\t%6.2f\n", i + 1, rgb[i][0], rgb[i][1], rgb[i][2]);
        fprintf(fp, "END_DATA\n");
        fclose(fp);
        return true;
    }

    
    // Writes CGATs file containing RGB and LAB fields only - No spectral data
    bool write_cgats_rgblab(const vector<V6>& rgblab, const string& filename, string descriptor)
    {
        FILE* fp = fopen(filename.c_str(), "wt");
        if (!fp)
            throw std::invalid_argument("File Open for Write Failed.");
        fprintf(fp,
            "CGATS.17\n"
            "NUMBER_OF_FIELDS 7\n"
            "BEGIN_DATA_FORMAT\n"
            "SAMPLE_ID RGB_R RGB_G RGB_B LAB_L LAB_A LAB_B\n"
            "END_DATA_FORMAT\n"
            "NUMBER_OF_SETS %d\n"
            "BEGIN_DATA\n", static_cast<int>(rgblab.size()));

        for (unsigned int i = 0; i < rgblab.size(); i++)
            fprintf(fp, "%d\t%6.2f\t%6.2f\t%6.2f\t%6.2f\t%6.2f\t%6.2f\n", i + 1,
                rgblab[i][0], rgblab[i][1], rgblab[i][2],
                rgblab[i][3], rgblab[i][4], rgblab[i][5]);
        fprintf(fp, "END_DATA\n");
        fclose(fp);
        return true;
    }

    // separate out RGB and LAB vectors, use auto [rgb, lab] = separate_rgb_lab(rgblab)
    pair<vector<V3>, vector<V3>> separate_rgb_lab(const vector<V6>& rgblab)
    {
        pair<vector<V3>, vector<V3>> ret;
        ret.first.reserve(rgblab.size()); ret.second.reserve(rgblab.size());
        for (auto x : rgblab)
        {
            ret.first.emplace_back(V3{ x[0], x[1], x[2] });
            ret.second.emplace_back(V3{ x[3], x[4], x[5] });
        }
        return ret;
    }

    // Combine rgb and lab vectors
    vector<V6> combine_rgb_lab(const vector<V3>& rgb, const vector<V3>& lab)
    {
        if (rgb.size() != lab.size())
            throw std::runtime_error("rgb and lab vectors must be same size");
        vector<V6> ret;
        ret.reserve(rgb.size());
        for (unsigned int i = 0; i < rgb.size(); i++)
            ret.emplace_back(V6{ rgb[i][0], rgb[i][1], rgb[i][2], lab[i][0], lab[i][1] , lab[i][2] });
        return ret;
    }

    // add the last 3 values in V6 (Lab values) to average later, check the first 3 are the same
    inline V6 add_lab(V6 arg1, V6 arg2) {
        for (int i = 0; i < 3; i++)
            if (arg1[i] != arg2[i])
                throw std::runtime_error("averaging LAB values requires RGB values to be identical");
        for (int i = 3; i < 6; i++)
            arg1[i] += arg2[i];
        return arg1;
    }


    // predicate for sort by RGB values
    bool less_than(const V6& arg1, const V6& arg2) {
        for (int i = 0; i < 3; i++)
        {
            if (arg1[i] < arg2[i])
                return true;
            else if (arg1[i] > arg2[i])
                return false;
        }
        return false;
    }

    // Overrides operator== for array<> to look at only the first 3 values
    bool operator==(const V6& arg1, const V6& arg2) {
        for (int i = 0; i < 3; i++)
            if (arg1[i] != arg2[i])
                return false;
        return true;
    }
}
