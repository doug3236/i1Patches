#define _CRT_SECURE_NO_WARNINGS
#include "i1isis.h"
#include "font.h"

using namespace cgats_utilities;
using namespace std::literals;

// make diamond image for row registration
inline Array2D<RGB> make_diamond() {
    Array2D<RGB> diamond(67, 133);
    diamond.fill({ 255,255,255 });
    for (int i = 0; i < 133 / 2; i++)
    {
        for (int ii = 0; ii < i / 2 + 2; ii++)
        {
            diamond(33 - ii, i) = RGB{ 0,0,0 };
            diamond(33 + ii, i) = RGB{ 0,0,0 };
            diamond(33 - ii, 132 - i) = RGB{ 0,0,0 };
            diamond(33 + ii, 132 - i) = RGB{ 0,0,0 };
        }
    }
    diamond.fill(RGB{ 0,0,0 }, 0, 67, 66, 1);
    return diamond;
}


// create 1 or more i1iSis ready tif images for i1Profiler scanning
// and create CGATS RGB file for loading into i1Profiler
void make_i1isis(const vector<RGB>& patch_list_all, const string& imagename, int page_count)
{
    int patch_count = int(patch_list_all.size()) / page_count;
    if (patch_count * page_count != patch_list_all.size())
        throw "Total patches must be multiple of page_count";
    for (int page_num = 0; page_num < page_count; page_num++)
    {
        vector<RGB> patch_list(patch_list_all.begin() + page_num * patch_count, patch_list_all.begin() + (page_num + 1) * patch_count);
        Font characters;
        // 6mm patch size in 600DPI, patches=142 px sqr
        const int col_count = 29;
        const int row_count = int(patch_list.size()) / 29;
        const int patch_size = 142;
        const int mm_6 = 142;
        // image offsets
        const int patch_row_offset = int((8.5 * 600) - (col_count * patch_size)) / 2;
        const int patch_col_offset = int((11.0 * 600) - (row_count * patch_size)) / 2;
        const int diamond_voffset = patch_size / 2 - 33;
        const int diamond_left_offset = patch_row_offset - (133 + 110);
        const int diamond_right_offset = patch_row_offset + col_count * patch_size + 110;
        Array2D<RGB> diamond = make_diamond();
        Array2D<RGB> page(int(11.0 * 600), int(8.5 * 600), RGB{ 255,255,255 });
        for (int row = 0; row < row_count; row++)
        {
            for (int col = 0; col < col_count; col++)
            {
                {
                    Array2D<RGB> val{ patch_size, patch_size, patch_list[row + col * row_count] };
                    page.insert(val, patch_col_offset + row * patch_size, patch_row_offset + col * patch_size);
                }
            }
            page.insert(diamond, patch_col_offset + row * patch_size + diamond_voffset, diamond_left_offset);
            page.insert(diamond, patch_col_offset + row * patch_size + diamond_voffset, diamond_right_offset);
        }


        Array2D<RGB> bar(mm_6, patch_size * col_count);
        page.insert(bar, patch_col_offset - 2 * mm_6, patch_row_offset);
        page.insert(bar, patch_col_offset + mm_6 + row_count * patch_size, patch_row_offset);
        string s = (page_count==1 ? "" : std::to_string(page_num + 1) + "-") + imagename + ".tif";
        auto label = s;
        if (label.length() > 80)
            label = label.substr(0, 80);
        page.insert(Font().get_line(label, 2), 150, 200);
        TiffWrite(s.c_str(), page);
    }
}

// create 1 or more i1iSis ready tif images for i1Profiler scanning
// and create txf and CGATS RGB file for loading into i1Profiler
void make_i1pro2(const vector<RGB>& patch_list_all, const string& imagename, int page_count)
{
    const int patch_count = 609;
    validate((int(patch_list_all.size()) / page_count) % 609 == 0, "I1Pro2 page size must be 609 patches");
    for (int page_num = 0; page_num < page_count; page_num++)
    {
        vector<RGB> patch_list(patch_list_all.begin() + page_num * patch_count, patch_list_all.begin() + (page_num + 1) * patch_count);
        Font characters;
        // 8mm patch size in 600DPI, patches=189 px sqr
        const int col_count = 29;
        const int row_count = 21;
        const int patch_size = 189;
        const int um_750 = 18;  // just under 2% larger at 600 DPI
        // image offsets
        const int patch_row_offset = int((11.0 * 600) - (col_count * patch_size)) / 2;
        const int patch_col_offset = int((8.5 * 600) - (row_count * patch_size)) / 2;
        Array2D<RGB> page(int(8.5 * 600), int(11.0 * 600), RGB{ 255,255,255 });
        for (int row = 0; row < row_count; row++)
        {
            for (int col = 0; col < col_count; col++)
            {
                {
                    Array2D<RGB> val{ patch_size, patch_size, patch_list[row + col * row_count] };
                    page.insert(val, patch_col_offset + row * patch_size, patch_row_offset + col * patch_size);
                }
            }
            //page.insert(diamond, patch_col_offset + row * patch_size + diamond_voffset, diamond_left_offset);
            //page.insert(diamond, patch_col_offset + row * patch_size + diamond_voffset, diamond_right_offset);
        }
        Array2D<RGB> bar(patch_size * row_count, um_750);
        page.insert(bar, patch_col_offset, patch_row_offset - um_750);
        page.insert(bar, patch_col_offset, patch_row_offset + col_count * patch_size);
        string s = (page_count == 1 ? "" : std::to_string(page_num + 1) + "-") + imagename + ".tif";
        auto label = s;
        // 50 pixels height, 30 pixels long per char * scale
        if (label.length() > 80)
            label = label.substr(0, 80);
        page.insert(Font().get_line(label, 2), 150, 200);

        page.insert(Font().get_line("------", 1), 538, 350);
        page.insert(Font().get_line("------", 1), 538, um_750 + patch_col_offset + col_count * patch_size);
        for (int i = 1; i <= 21; i++)
        {
            string s = std::to_string(i);
            string se = s;
            while (s.length() < 6)
            {
                s = s + "-";
                se = string("-") + se;
            }
            page.insert(Font().get_line(s, 1), 538+i*patch_size, 350);
            page.insert(Font().get_line(se, 1), 538 + i * patch_size, um_750 + patch_col_offset + col_count * patch_size);
        }
        TiffWrite(s.c_str(), page);
    }
}

void make_isis_txf(const vector<RGB>& patch_list_all, const string& imagename, int page_count)
{
    const int col = 29;
    size_t rows = (patch_list_all.size() / page_count)/29;
    double margin = 5.0 + 6.0*(33.0 - rows) / 2.0;
    size_t patches_per_page=29*rows;
    validate(rows * page_count * 29 == patch_list_all.size(), "patches not a multiple of rows, columns");
    FILE* fp = fopen((imagename+".txf"s).c_str(), "wt");
    fprintf(fp, txf_0);
    for (int i = 0; i < int(patch_list_all.size()); i++)
    {
        fprintf(fp, txf_1, i + 1, int(patch_list_all[i][0]), int(patch_list_all[i][1]), int(patch_list_all[i][2]));
            
    }
    fprintf(fp, txf_2, imagename.c_str(), margin, margin);
    fclose(fp);
}


void make_i1pro2_txf(const vector<RGB>& patch_list_all, const string& imagename, int page_count)
{
    const int col = 29;
    size_t rows = (patch_list_all.size() / page_count) / 29;
    validate(rows == 21, "rows must be exactly 21 for Pro2");
    size_t patches_per_page = 29 * rows;
    validate(rows * page_count * 29 == patch_list_all.size(), "patches not a multiple 609");
    FILE* fp = fopen((imagename + ".txf"s).c_str(), "wt");
    fprintf(fp, txf_0);
    for (int i = 0; i < int(patch_list_all.size()); i++)
    {
        fprintf(fp, txf_1, i + 1, int(patch_list_all[i][0]), int(patch_list_all[i][1]), int(patch_list_all[i][2]));

    }
    fprintf(fp, txf_2_Pro2, imagename.c_str());
    fclose(fp);
}