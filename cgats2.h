#pragma once

#include <vector>
#include <array>
#include <string>
#include <random>
#include <numeric>
#include <unordered_map>
#include <functional>
#include "cgats.h"
#include "statistics.h"
#include "deltae.h"

extern "C"
{
    // Use littleCMS to process LAB array to RGB device values, returns false if no profile
    int convert_device_rgb_to_lab_mem(const void* profilemem, int prof_len, const float rgb[][3], float lab[][3], int len, int colorimetric_mode);
    int convert_lab_to_device_rgb_mem(const void* profilemem, int prof_len, const float lab[][3], float rgb[][3], int len, int colorimetric_mode);
    int is_profile_valid(const char* profile);
}

using std::vector;
using std::array;
using std::string;
using std::pair;
using V3 = array<float, 3>;
using V6 = array<float, 6>;


template<class T>
std::vector<T> randomize(const std::vector<T>& vin, bool de_randomize = false, int seed = 1)
{
    std::vector<T> ret(size(vin));
    std::vector<int> seq(size(vin));
    std::mt19937 g(seed);
    std::iota(seq.begin(), seq.end(), 0);
    std::shuffle(seq.begin(), seq.end(), g);
    if (de_randomize == false)
    {
        auto p = vin.begin();
        for (int x : seq)
            ret[x] = *p++;
    }
    else
    {
        auto p = ret.begin();
        for (int x : seq)
            *p++ = vin[x];
    }
    return ret;
}


struct PatchSet {
    int dup;
    int pcount;
};


struct ProfList {
    ProfList(int argc, char** argv);
    int rows_per_page;
    vector<pair<string, vector<V3>>> rgb_set;    // filename/rgb base collection
};


// used to gather statistics on triplets such as L*a*b*
struct StatV3 {
    array<Statistics, 3> s3;
    int clk(const V3& v3) {
        s3[0].clk(v3[0]); s3[1].clk(v3[1]); s3[2].clk(v3[2]);
        return s3[0].n();
    }
    operator V3() { return { s3[0].ave(), s3[1].ave(), s3[2].ave() }; }
    V3 min() { return V3{ s3[0].min(), s3[1].min(), s3[2].min() }; }
    V3 max() { return V3{ s3[0].max(), s3[1].max(), s3[2].max() }; }
    V3 std() { return V3{ s3[0].std(), s3[1].std(), s3[2].std() }; }
    int n() { return s3[0].n(); }
};

//struct CGATS_Bidir;
// gathers CGATs file info
class CgatsMeasure {
public:
    CgatsMeasure() = default;
    CgatsMeasure(const string & cgatsfile, size_t set_number=1);
    ptrdiff_t rgb_loc{};  // location of RGB_R in fields
    ptrdiff_t lab_loc{};  // location of LAB_L in fields
    vector<vector<float>> lines_f{};  // floats of line numerical values in CGATs file (offset to RGB)
    vector<V3> getv_rgb() const;
    vector<V3> getv_lab() const;
    vector<V6> getv_rgblab() const;
    bool write_cgats(const string& filename);
private:
    string filename{};
    vector<vector<string>> lines{};   // tokenized lines in CGATs file
    vector<string> fields{};          // tokenized list of fields
    size_t data_offset{}; // lines[data_offset]== first line of data list
    int num_of_fields{};  // num_of_fields==fields.size()==data_ptr[0].size()
    int num_of_sets{};    // data_ptr[0] to data_ptr[num_of_sets-1]
};

// This is encoded into the first patches so that the filenames and sizes can be recovered
// when the patches are read.
struct PatchHeader
{
    bool header_valid{ false };
    bool is_i1pro2{};
    size_t patches_per_page;    // 957 on full, 33 row page
    size_t rows_per_page;       // 33 on full page
    size_t pages;               // number of printer pages in aggregated patch set
    size_t all_colors;          // number of check color patches in aggregated patch set
    size_t colors_per_page;     // colors per page on all pages
    size_t residual;            // extras, all_colors - colors_per_page * pages;
    size_t header_size;         // number of V3 elements in header
    vector<pair<string, size_t>> user_patch_info;
};

// Processes CGATs measurement files with a defined structure
// that included encoded header info in the initial RGB values
struct CGATS_Bidir
{
    CGATS_Bidir() = default;
    PatchHeader header;
    struct Stats {
        size_t index;
        float dE;
        array<float, 3> rgb_f, lab_f, lab_r;
    };
    CGATS_Bidir(string file, bool cgats_output, bool not_structured, bool make_argyll = false);
    string file_base;
    string file_tail;
    vector<Stats> stats;
    CgatsMeasure m_fwd;
    bool has_forward_and_reverse{};
    void print_random_patch_stats();
};

PatchHeader get_header(const vector<V3>& v_in);



// encode rows/patches in first V3 of combined patch file
// encode i1pro2 by creating header as if rows_per_page is 36 (0xf)
inline V3 encode_rows_and_patch_count(int rows_per_page, int total_patches, bool is_i1pro2)
{
    //validate(rows >= 21 && 33 >= rows, "rows per page must be between 21 and 33");
    int combo = rows_per_page - 21 | total_patches << 4;
    if (is_i1pro2)
        combo = 15 | total_patches << 4;
    V3 ret;
    ret[0] = float(combo % 256); combo = combo >> 8;
    ret[1] = float(combo % 256); combo = combo >> 8;
    ret[2] = float(combo % 256);
    return ret;
}

// encode rows in first V3 of combined patch file
inline pair<int,int> decode_rows_and_patch_count(V3 first_V3)
{
    int combo = int(first_V3[0] + 256.f*first_V3[1] + 65536.f*first_V3[2]);
    int rows_per_page = (combo & 15) + 21;
    int total_patches = (combo >> 4);
    return pair{ rows_per_page,total_patches};
}

extern std::vector<array<float, 3>> edge_colors;
extern std::array<std::array<float, 3>, 2> demark;

string get_profile_image(string profile_name);
