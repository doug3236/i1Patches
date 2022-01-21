/*
Copyright (c) <2020> <doug gray>

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

// Requires libtiff.  www.libtiff.org
// For some reason Adobe Photoshop Tiff files use a field type of
// 7 (UNDEFINED) for some extra metadata that needs to be ignored
// but may be printed. Patch the source or ignore the message.
// See line 1554 in tif_dirread.c

#include <tiffio.h>
#include <string>
#include <vector>
#include <array>
#include <cassert>
#include <cmath>
#include <numeric>
#include <future>
#include "array2d.h"

using std::array;
using std::vector;
using std::string;

using RGB = array<float, 3>;
//using RGB2D = vector<vector<RGB>>;

// Utility Functions
class ArrayRGB;
string get_profile_image(string profile_name);
string get_profile(int resource);
void attach_profile(const std::string & profile, TIFF * out, const ArrayRGB & rgb);
void TiffWrite(const char *file, const ArrayRGB &rgb, const std::string &profile);
void TiffWrite(const char* file, const Array2D<RGB>& rgb, const string& profile="");
ArrayRGB TiffRead(const char* filename, float gamma);
Array2D<RGB> TiffRead(const char* file);

static auto launchType = std::launch::deferred;

// Floating point RGB array representing an image including some context info
// RGB values are stored in separate vectors since operations on each are independant
// and so can be easily multi-threaded. Values are normally in gamma=1 and are [0:1]
class ArrayRGB {        // From top left, r->down, c->right 
public:
    std::vector<float> v[3];			// separate vectors for each R, G, and B channels
	std::vector<uint8> profile;     // size is zero if no profile attached to image
    int dpi;
    int nc, nr;
    float gamma;
    bool from_16bits;
    ArrayRGB(int NR = 0, int NC = 0, int DPI=0, bool bits16=true, float gamma=1.7)
        : nc(NC), nr(NR), dpi(DPI), from_16bits(bits16),
          gamma(gamma) { for (auto& x:v) x.resize(NR*NC); }
    void resize(int nrows, int ncols) { nr = nrows; nc = ncols; for (auto& x:v) x.resize(nc*nr); }
    void fill(float red, float green, float blue);
    void copy(const ArrayRGB &from, int offsetx, int offsety);
    ArrayRGB subArray(int rs, int re, int cs, int ce);	// rs:row start, re: row end, etc.
    void copyColumn(int to, int from);
    void copyRow(int to, int from);
	float& operator()(int r, int c, int color) { return v[color][r*nc+c]; };
    float const & operator()(int r, int c, int color) const {return v[color][r*nc+c];}
    void scale(float factor);    // scale all array values by factor


};

