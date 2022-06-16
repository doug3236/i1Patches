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
#define _CRT_SECURE_NO_WARNINGS

#include "tiffresults.h"
#include <memory>
#include <array>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "array2d.h"
#include "mainutils.h"

using std::vector;
using std::array;
using std::ios_base;
using std::ios;
using std::string;
using std::ifstream;
using std::tuple;


// Reads a tiff file and returns image in linear space (gamma=1) scaled 0-1
ArrayRGB TiffRead(const char *filename, float gamma)
{
    ArrayRGB rgb;               // ArrayRGB to be returned
    uint32 prof_size = 0;       // size of byte arrray for storing profile if present
    uint8 *prof_data = nullptr; // ptr to byte array
    uint16 bits;                // image was from 8 or 16 bit tiff
    uint32 height;              // image pixe sizes
    uint32 width;
    uint32 size;                // width*height
    uint16 planarconfig;        // pixel tiff storage orientation
    float local_dpi;

    vector<uint32> image;
    TIFF *tif;

    tif = TIFFOpen(filename, "r");
    if (tif == 0)
    {
        return rgb;
    }
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits);
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tif, TIFFTAG_XRESOLUTION, &local_dpi);       // assume Xand Y the same
    TIFFGetField(tif, TIFFTAG_ICCPROFILE, &prof_size, &prof_data);
    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarconfig);
    if (prof_size!=0)
    {
        rgb.profile.resize(prof_size);
        memcpy(rgb.profile.data(), prof_data, prof_size);
        
        // code to extract ICC profiles
        //FILE *fp=fopen("test.icm", "wb");
        //fwrite(prof_data, 1, prof_size, fp);
        //fclose(fp);
    }

    size = width*height;
    rgb.resize(height, width);
    rgb.nc = width;
    rgb.nr = height;
    rgb.dpi = (int)local_dpi;
    rgb.gamma = gamma;
    if (planarconfig != PLANARCONFIG_CONTIG || bits == 8) {
        if (bits == 16)
            std::cout << "16 bit tif file not recognized, reverting to 8 bit read.\n";
        rgb.from_16bits = false;
        image.resize(height*width);
        int istatus = TIFFReadRGBAImage(tif, width, height, image.data());
        if (istatus==1) {
            for (uint32 c = 0; c < width; c++)
            {
                for (uint32 r = 0; r < height; r++)
                {
                    int yt = height-r-1;
                    //auto z0 = this->operator[](yt)[c];
                    uint32 z0 = image[yt*width+c];
                    rgb(r, c, 0) = pow(static_cast<float>(z0 & 0xff)/255, gamma);
                    rgb(r, c, 1) = pow(static_cast<float>((z0>>8) & 0xff)/255, gamma);
                    rgb(r, c, 2) = pow(static_cast<float>((z0>>16) & 0xff)/255, gamma);
                }
            }
        }
        else
            throw "Bad TIFFReadRGBAImage";
    }
    else
    {
        rgb.from_16bits = true;
        uint32 row, col;
        uint16 config, nsamples;

        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nsamples);
        vector<uint16_t> bufs(TIFFScanlineSize(tif));
        if (config == PLANARCONFIG_CONTIG) {
            for (row = 0; row < height; row++)
            {
                tmsize_t count = TIFFReadScanline(tif, bufs.data(), row);
                for (col = 0; col < width; col++)
                {
                    rgb(row, col, 0) = pow(1.0f*bufs[col*nsamples + 0]/65535, gamma);
                    rgb(row, col, 1) = pow(1.0f*bufs[col*nsamples + 1] / 65535, gamma);
                    rgb(row, col, 2) = pow(1.0f*bufs[col*nsamples + 2] / 65535, gamma);
                }
            }
        }
        else
            throw "16 bit file type not supported";
    }
    return rgb;
}

void TiffWrite(const char* file, const Array2D<RGB>& rgb, const string& profile)
{
    ArrayRGB rgb3(rgb.nr, rgb.nc, global_modifiers::expand_tif_2_percent ? 588 : 600, false, 1.0);
    for (int i = 0; i < rgb.nr; i++)
        for (int ii = 0; ii < rgb.nc; ii++)
        {
            rgb3(i, ii, 0) = rgb(i, ii)[0] / 255.f;
            rgb3(i, ii, 1) = rgb(i, ii)[1] / 255.f;
            rgb3(i, ii, 2) = rgb(i, ii)[2] / 255.f;
        }
    TiffWrite(file, rgb3, profile);
}

Array2D<RGB> TiffRead(const char* file)
{
    auto rgb = TiffRead(file, 1);
    Array2D<RGB> ret(rgb.nr, rgb.nc);
    for (int i = 0; i < rgb.nr; i++)
        for (int ii = 0; ii < rgb.nc; ii++)
        {
            ret(i, ii)[0] = rgb(i, ii, 0) * 255;
            ret(i, ii)[1] = rgb(i, ii, 1) * 255;
            ret(i, ii)[2] = rgb(i, ii, 2) * 255;
        }
    return ret;
}


void TiffWrite(const char *file, const ArrayRGB &rgb, const string &profile)
{
    float gamma = rgb.gamma;
    int sampleperpixel=3;
    TIFF *out = TIFFOpen(file, "w");
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, rgb.nc);  // set the width of the image
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, rgb.nr);    // set the height of the image
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
                                                                    //   Some other essential fields to set that you do not have to understand for now.
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(out, TIFFTAG_XRESOLUTION, (float)rgb.dpi);
    TIFFSetField(out, TIFFTAG_YRESOLUTION, (float)rgb.dpi);
    TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_ADOBE_DEFLATE);
    //attach_profile(profile, out, rgb);      // attach profile if provided. May be image or filename. Empty=no profile
    if (!rgb.from_16bits)
    {
        TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
        tsize_t linebytes = sampleperpixel * rgb.nc;
        vector<unsigned char> buf(linebytes);

        // We set the strip size of the file to be size of one row of pixels
        TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, rgb.nc*sampleperpixel));
        vector<array<uint8, 3>> image(rgb.nr*rgb.nc);
        //unique_ptr<uint32[]> image(new uint32[rgb.nr*rgb.nc]);
        auto igamma = 1 / gamma;
        auto one_channel_to_8 = [](const vector<float> &image_ch, int cols, float inv_gamma) {
            float resid = 0;
            vector<uint8> ret(image_ch.size());
            for (size_t i = 0; i < ret.size(); i++)
            {
                if (i % cols == 0)    // No offset at start of each row
                    resid = 0;
                float tmp = inv_gamma==1.0f ? 255* image_ch[i] : 255 * pow(image_ch[i], inv_gamma);
                if (tmp > 255) tmp = 255;
                if (tmp < 0) tmp = 0;
                uint8 tmpr = static_cast<uint8>(tmp + .5);
                resid += tmp - tmpr;
                if (resid > .5 && tmpr < 255)
                {
                    resid -= 1;
                    tmpr++;
                }
                else if (resid < -.5)
                {
                    resid += 1;
                    tmpr--;
                }
                ret[i] = tmpr;
            }
            return ret;
        };
        vector<uint8> red = one_channel_to_8(rgb.v[0], rgb.nc, igamma);
        vector<uint8> green = one_channel_to_8(rgb.v[1], rgb.nc, igamma);
        vector<uint8> blue = one_channel_to_8(rgb.v[2], rgb.nc, igamma);

        for (int r = 0; r < rgb.nr; r++)
            for (int c = 0; c < rgb.nc; c++)
            {
                int offset = r * rgb.nc + c;
                //image[offset] = red[offset] | green[offset] << 8 | blue[offset] << 16 | 0xff000000;
                image[offset][0] = red[offset];
                image[offset][1] = green[offset];
                image[offset][2] = blue[offset];
            }

        //Now writing image to the file one strip at a time
        for (int row = 0; row < rgb.nr; row++)
        {
            //memcpy(buf, &image[row*linebytes/sizeof(uint32)], linebytes);    // check the index here, and figure out why not using h*linebytes
            memcpy(buf.data(), &image[row*rgb.nc], linebytes);    // check the index here, and figure out why not using h*linebytes
            if (TIFFWriteScanline(out, buf.data(), row, 0) < 0)
                break;
        }
        TIFFClose(out);
    }
    else
    {
        TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
        // 16  bit write
        vector<array<uint16, 3>> buf(rgb.nc);
        // We set the strip size of the file to be size of one row of pixels
        TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, rgb.nc*sampleperpixel));

        auto igamma = 1 / gamma;

        for (int r = 0; r < rgb.nr; r++)
        {
            for (int c = 0; c < rgb.nc; c++)
            {
                for (int color = 0; color < 3; color++)
                {
                    buf[c][color] = static_cast<uint16>(pow(std::clamp(rgb(r, c, color), 0.f, 1.f), igamma) * 65535);
                }
            }
            if (TIFFWriteScanline(out, buf.data(), r, 0) < 0)
                throw "Error writing tif";
        }
        TIFFClose(out);
    }
}



void ArrayRGB::fill(float red, float green, float blue) {
    for (auto& x:v[0]) { x = red; }
    for (auto& x:v[1]) { x = green; }
    for (auto& x:v[2]) { x = blue; }
}

void ArrayRGB::copy(const ArrayRGB &from, int offsetx, int offsety)
{
    if (from.nr + offsetx > nr || from.nc + offsety > nc)
        throw std::invalid_argument("copy with out of range arguments");
    //assert(from.nr + offsetx <= nr);
    //assert(from.nc + offsety <= nc);
    for (int color = 0; color < 3; color++)
        for (int x = 0; x < from.nr; x++)
            for (int y = 0; y < from.nc; y++)
                (*this)(x+offsetx, y+offsety, color) = from(x, y, color);
}

// note: re (row end) and ce (column end) are included, not one past
ArrayRGB ArrayRGB::subArray(int rs, int re, int cs, int ce)
{
    if (!(rs <= re && re < nr) || !(cs <= ce && ce < nc))
        throw std::invalid_argument("copy with out of range arguments");
    //assert(rs <= re && re < nr);
    //assert(cs <= ce && ce < nc);
    ArrayRGB s(re-rs+1, ce-cs+1);
    for (int color = 0; color < 3; color++)
        for (int r = rs; r <= re; r++)
            for (int c = cs; c <= ce; c++)
                s(r-rs, c-cs, color) = (*this)(r, c, color);
    return s;
}

void ArrayRGB::copyColumn(int to, int from)
{
    for (int color = 0; color < 3; color++)
        for (int r = 0; r < nr; r++)
            (*this)(r, to, color) = (*this)(r, from, color);
}

void ArrayRGB::copyRow(int to, int from)
{
    for (int color = 0; color < 3; color++)
        for (int c = 0; c < nc; c++)
            (*this)(to, c, color) = (*this)(from, c, color);
}

void ArrayRGB::scale(float factor)    // scale all array values by factor
{
    for (int i = 0; i < 3; i++)
        for (auto &x:v[i])
            x *= factor;
}


