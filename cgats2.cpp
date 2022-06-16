#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <utility>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <tuple>
#include <set>
#include <string>
#include "cgats2.h"
#include "checkcolors.h"
#include "drift.h"
#include "mainutils.h"

using namespace std::string_literals;

std::vector<array<float,3>> edge_colors{ {0,255,255},{255,255,0},{255,0,255},{180,255,255},{255,255,180},{255,180,255},
                {50,50,50}, {100,100,100}, {150,150,150}, {200,200,200} };

std::array<std::array<float, 3>, 2> demark{ {70,126,199,  218,247,253} };

using std::vector;
using std::array;
using std::string;
using std::pair;

// tokenize a line and return vector of tokens, token may be quoted
inline vector<string> parse(const string& s) {
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
        //if (line_words.size() != 0)             // uncomment to skip empty lines
        ret.push_back(line_words);
    }
    return ret;
}

CgatsMeasure::CgatsMeasure(const string& cgatsfile, size_t set_number)
{
    filename = cgatsfile;
    lines = tokenize_file(filename);
    while (set_number > 1 && lines.size() > 0)
    {
        for (size_t i = 0; i < lines.size(); i++)
            if (lines[i].size()==1 && lines[i][0] == "END_DATA")
            {
                lines.erase(lines.begin(), lines.begin() + i + 1);
                set_number--;
                break;
            }
    }
    if (lines.size() < 5)
        return;
    num_of_fields = std::stoi(std::find_if(lines.begin(), lines.end(),
        [](const vector<string>& arg) {return arg.size()!=0 && arg[0] == "NUMBER_OF_FIELDS"; })[0][1]);
    num_of_sets = std::stoi(std::find_if(lines.begin(), lines.end(),
        [](const vector<string>& arg) {return arg.size() != 0 && arg[0] == "NUMBER_OF_SETS"; })[0][1]);

    fields = std::find_if(lines.begin(), lines.end(),
        [](const vector<string>& arg) {return arg.size() != 0 && arg[0] == "BEGIN_DATA_FORMAT"; })[1];
    validate(num_of_fields == fields.size(), "Bad CGATS Format, num of fields <> field count");
    data_offset = (std::find_if(lines.begin(), lines.end(),
        [](const vector<string>& arg) {return arg.size() != 0 && arg[0] == "BEGIN_DATA"; }) + 1) - lines.begin();

    // Note: if RGB_R or LAB_L don't exist returns data.fields.size()
    rgb_loc = std::find_if(fields.begin(), fields.end(),
        [](const string& arg) {return arg == "RGB_R"; }) - fields.begin();
    lab_loc = std::find_if(fields.begin(), fields.end(),
        [](const string& arg) {return arg == "LAB_L"; }) - fields.begin();

    for (size_t row = 0; row < num_of_sets; row++)
    {
        vector<float> tmp(num_of_fields);
        for (size_t col = rgb_loc; col < num_of_fields; col++)
            tmp[col] = std::stof(lines[row+data_offset][col]);
        lines_f.push_back(tmp);
    }
    if (is_suffix_ti1(filename) && fields[1]=="RGB_R")  // if Argyll .ti1 patch file make sure RGB starts at offset 1
    {
        for (size_t row = 0; row < num_of_sets; row++)
            for (size_t i = 1; i < 4; i++)
                if (global_modifiers::fractional)
                    lines_f[row][i] = lines_f[row][i] * 255 / 100;  // rescale RGB to [0:255]
                else
                    lines_f[row][i] = std::round(lines_f[row][i] * 255 / 100);  // round/rescale RGB to [0:255]
    }
}

vector<V3> CgatsMeasure::getv_rgb() const
{
    vector<V3> ret; ret.reserve(lines_f.size());
    for (size_t row = 0; row < lines_f.size(); row++)
    {
        auto p = &lines_f[row][0] + rgb_loc;
        ret.push_back({ p[0], p[1], p[2] });
    }
    return ret;
}
vector<V3> CgatsMeasure::getv_lab() const
{
    vector<V3> ret; ret.reserve(lines_f.size());
    for (size_t row = 0; row < lines_f.size(); row++)
    {
        auto p = &lines_f[row][0] + lab_loc;
        ret.push_back({ p[0], p[1], p[2] });
    }
    return ret;
}
vector<V6> CgatsMeasure::getv_rgblab() const
{
    vector<V6> ret; ret.reserve(lines_f.size());
    for (size_t row = 0; row < lines_f.size(); row++)
    {
        auto p_rgb = &lines_f[row][0] + rgb_loc;
        auto p_lab = &lines_f[row][0] + lab_loc;
        ret.push_back({ p_rgb[0], p_rgb[1], p_rgb[2], p_lab[0], p_lab[1], p_lab[2] });
    }
    return ret;
}


bool CgatsMeasure::write_cgats(const string& filename)
{
    FILE* fp = fopen(filename.c_str(), "wt");
    if (!fp)
        throw std::invalid_argument("File Open for Write Failed.");
    for (size_t row = 0; row < lines_f.size()+data_offset; row++)
    {
        auto data_row = int(row) - int(data_offset);
        if (data_row < 0 && lines[row].size() == 0)
            fprintf(fp, "\n");
        else if (data_row < 0 && lines[row].size() == 1)
        {
            fprintf(fp, "%s\n", lines[row][0].c_str());
        }
        else if (data_row < 0 && lines[row].size() == 2)
        {
            if (lines[row][0] == "NUMBER_OF_FIELDS")
                fprintf(fp, "%s\t%s\n", lines[row][0].c_str(), lines[row][1].c_str());
            else if (lines[row][0] == "NUMBER_OF_SETS")
                fprintf(fp, "%s\t%s\n", lines[row][0].c_str(), std::to_string(lines_f.size()).c_str());
            else if (lines[row][0] == "DESCRIPTOR")
                fprintf(fp, "%s\t\"%s\"\n", lines[row][0].c_str(), filename.c_str());
            else
                fprintf(fp, "%s\t\"%s\"\n", lines[row][0].c_str(), lines[row][1].c_str());
        }
        else if (int(row) - int(data_offset) < 0)
        {
            for (size_t col = 0; col < lines[row].size() - 1; col++)
                fprintf(fp, "%s\t", lines[row][col].c_str());
            fprintf(fp, "%s\n", lines[row].back().c_str());
        }
        else
        {
            auto fmt2 = [](double v) {
                char buf[10];
                sprintf(buf, "%8.2f", v);
                return string(buf);
            };
            auto fmt4 = [](double v) {
                char buf[10];
                sprintf(buf, "%8.4f", v);
                return string(buf);
            };
            auto fmt5 = [](double v) {
                char buf[10];
                sprintf(buf, "%8.5f", v);
                return string(buf);
            };
            //if (num_of_sets == lines_f.size())
            if (data_row >= 0)
            {
                for (int col = 0; col < lines_f[data_row].size() - 1; col++)
                    if (col < rgb_loc)
                        if (num_of_sets == lines_f.size())
                            fprintf(fp, "%s\t", lines[row][col].c_str());
                        else
                            fprintf(fp, "%d\t", data_row + 1);
                    else if (col >= rgb_loc + 3 && col < lab_loc)
                        fprintf(fp, "%s\t", fmt5(lines_f[data_row][col]).c_str());
                    else if (col >= lab_loc + 3)
                        fprintf(fp, "%s\t", fmt4(lines_f[data_row][col]).c_str());
                    else
                        fprintf(fp, "%s\t", fmt2(lines_f[data_row][col]).c_str());
                fprintf(fp, "%s\n", fmt2(lines_f[data_row].back()).c_str());
            }
        }

        if (data_row+1 == lines_f.size())
        {
            fprintf(fp, "END_DATA\n");
            break;
        }
    }
    fclose(fp);
    return true;
}


// get length of header, and vector of <file name, size>, all in V3 units
    //   3 bytes per row and patch counts excluding check colors
    //   number of embedded files: 1 byte (at second patch's "r" values)
    //   Repeat these two for each entry
    //      RGB File Names Head: (variable, each name null terminated)
    //      Length of RGB file in bytes: 2
    //   null fill to multiple of 3 bytes
PatchHeader get_header(const vector<V3>& v_in)
{
    PatchHeader retx;
    vector<V3> v = v_in;
    auto [rows, len] = decode_rows_and_patch_count(v[0]);
    if (rows == 36)     // indicates the measurement file is from an i1Pro2
    {
        rows = 21;
        retx.is_i1pro2 = true;
        v = change_i1_order(v, rows, 29, false);
    }
    if (v.size() % rows != 0)       // must be multiple of rows. Exit if invalid header
        return retx;
    retx.patches_per_page = rows * 29;
    retx.rows_per_page = rows;
    retx.pages = v.size() / (retx.patches_per_page);
    if (retx.pages * retx.patches_per_page != v.size()) // must match or invalid header
        return retx;
        
    retx.all_colors = v.size() - len;
    retx.colors_per_page = retx.all_colors / retx.pages;
    retx.residual = retx.all_colors - (retx.colors_per_page * retx.pages);
    int entries_count = static_cast<int>(v[1][0]);
    size_t offset = 4;
    vector<pair<string, size_t>> ret(entries_count);
    for (int entry = 0; entry < entries_count; entry++, offset++)
    {
        string s;
        for (; v[offset / 3][offset % 3] != 0; offset++)
        {
            s = s + static_cast<char>(v[offset / 3][offset % 3]);
            if (s.length() > 30)    // no name larger than 30
                return retx;
        }
        ret[entry].first = s;
    }
    size_t total_size = 0;
    for (int entry = 0; entry < entries_count; entry++, offset += 2)
    {
        // access patch RGB values 2 bytes at a time
        ret[entry].second = static_cast<size_t>(v[offset / 3][offset % 3] + 256.0f * v[(offset + 1) / 3][(offset + 1) % 3]);
        total_size += ret[entry].second;
    }
    retx.user_patch_info = ret;
    retx.header_size = (offset - 1) / 3 + 1;
    if (total_size + retx.header_size != len)        // cross check sizes
        return retx;
    retx.header_valid = true;
    return retx;
}

CGATS_Bidir::CGATS_Bidir(string file, bool cgats_output, bool not_structured, bool make_argyll)
{
    validate(is_cgats_measurement(file), "Must be measurement file, *_Mn.txt");
    file_tail = file.substr(file.length() - 7, file.length());
    string tail = file_tail;
    file_base = file.substr(0, file.length() - 7);
    m_fwd = CgatsMeasure(file_base + file_tail);

    // Section which reads reverse scanned file if present.
    //   Reverse file is the same name but with "r" appended priot to the _MN.txt suffix
    //      "measurement_file_M2.txt" and "measurement_filer_M2.txt"
    CgatsMeasure m_rev;
    try { m_rev = CgatsMeasure(file_base + "r" + file_tail); }
    catch ([[maybe_unused]] std::exception& e) {};
    if (m_rev.lines_f.size()!=0)
    {
        validate(m_fwd.lines_f.size() == m_rev.lines_f.size(), "measurement file sizes differ");
        validate(m_fwd.lines_f[0].size() == m_rev.lines_f[0].size(), "measurement file data lines differ");
        Statistics dEStat;
        for (size_t row = 0; row < m_fwd.lines_f.size(); row++)
        {
            Stats stat;
            stat.index = row;
            float* prgb_f = &m_fwd.lines_f[row][m_fwd.rgb_loc];
            float* prgb_r = &m_rev.lines_f[m_rev.lines_f.size() - row - 1][m_rev.rgb_loc];
            validate(prgb_f[0] == prgb_r[0] && prgb_f[1] == prgb_r[1] && prgb_f[2] == prgb_r[2],
                "Patch target scan RGB's don't match");
            float* plab_f = &m_fwd.lines_f[row][m_fwd.lab_loc];
            float* plab_r = &m_rev.lines_f[m_rev.lines_f.size() - row - 1][m_rev.lab_loc];
            stat.rgb_f = array{ prgb_f[0], prgb_f[1], prgb_f[2] };
            stat.lab_f = array{ plab_f[0], plab_f[1], plab_f[2] };
            stat.lab_r = array{ plab_r[0], plab_r[1], plab_r[2] };
            stat.dE = float(sqrt(pow(plab_f[0] - plab_r[0], 2) + pow(plab_f[1] - plab_r[1], 2) + pow(plab_f[2] - plab_r[2], 2)));
            dEStat.clk(stat.dE);
            stats.push_back(stat);
            for (size_t col = 0; col < m_fwd.lines_f[row].size(); col++)
            {
                m_fwd.lines_f[row][col] += m_rev.lines_f[m_rev.lines_f.size() - row - 1][col];
                m_fwd.lines_f[row][col] /= 2.0f;
            }
        }
        std::sort(stats.begin(), stats.end(), [](Stats& a, Stats& b) {return a.dE > b.dE; });
        has_forward_and_reverse = true;
    }
    {
        V3 lab0{}, lab255{};
        int cnt0{}, cnt255{};
        auto rgb = m_fwd.getv_rgb();
        auto lab = m_fwd.getv_lab();
        for (size_t i = 0; i < rgb.size(); i++)
        {
            if (rgb[i] == V3{ 0,0,0 })
            {
                cnt0++;
                lab0 = lab0 + lab[i];
            }
            if (rgb[i] == V3{ 255,255,255 })
            {
                cnt255++;
                lab255 = lab255 + lab[i];
            }
        }
        if (cnt0 > 0 && cnt255 > 0)
        {
            printf("\nFile: %s\n", file.c_str());
            for (auto& x : lab0) x /= cnt0;
            for (auto& x : lab255) x /= cnt255;
            printf("%d black patches, ave Lab: %5.2f %5.2f %5.2f\n"
                "%d white patches, ave Lab: %5.2f %5.2f %5.2f\n",
                cnt0, lab0[0], lab0[1], lab0[2], cnt255, lab255[0], lab255[1], lab255[2]);
            printf("\n");
        }
    }
    if (not_structured)
        return;

    // Get the encoded header info and added check colors along edge
    header = get_header(m_fwd.getv_rgb());
    if (!header.header_valid)
        return;
    if (header.is_i1pro2)
    {
        m_fwd.lines_f = change_i1_order(m_fwd.lines_f, 21, 29, false);
        printf("I1Pro2 structure: ");
    }
    printf("%zd rows, %zd pages\n", header.rows_per_page,
        header.pages);

    vector<vector<float>> core;
    for (size_t page = 0; page < header.pages; page++)
    {
        size_t color_check_start = header.patches_per_page - header.colors_per_page;
        if (header.residual != 0)
        {
            color_check_start--;
            header.residual--;
        }
        for (size_t i = 0; i < header.patches_per_page; i++)
            if (i < color_check_start)
            {
                core.push_back(m_fwd.lines_f[page * header.patches_per_page + i]);
            }
            else
            {
                float* pa = &m_fwd.lines_f[page * header.patches_per_page + i][m_fwd.rgb_loc];
                float* pb = &edge_colors[(i + (10 - header.patches_per_page % 10)) % 10][0];
                validate(pa[0] == pb[0] && pa[1] == pb[1] && pa[2] == pb[2], "Check colors mismatch");
            }
    }

    // core contains measurement data excluding all color check patches
    core.erase(core.begin(), core.begin() + header.header_size);  // remove header
    core = randomize(core, true);                           // de-randomize
        
    // extract and write files
    auto& set = header.user_patch_info;
    auto p = core.begin(); // p=pointer to start of profile patches
    std::multiset<string> names;
    for (auto s : set)
        names.insert(s.first);
    for (auto iter = set.begin(); iter < set.end(); iter++)
        if (names.count(iter->first) > 1)
        {
            string name_base = iter->first;
            char c = 'a';
            for (auto iter_s = iter; iter_s < set.end(); iter_s++)
                if (name_base == iter_s->first)
                    iter_s->first += "_"s + c++;
        }

    printf("\nExtracting Measurement Files: ");
    FILE* fp = nullptr;
    if (make_argyll && cgats_output == true)
    {
        fp = fopen("argyll.bat", "wt");
        fprintf(fp, "set ARGYLL_CREATE_WRONG_VON_KRIES_OUTPUT_CLASS_REL_WP=1\n");
    }
    for (size_t i = 0; i < set.size(); i++)
    {
        printf("%s ", (set[i].first + file_tail).c_str());
        if (cgats_output == true)
        {
            //cout << names.count(set[i].first) << "\n";
            auto tmp = m_fwd;
            tmp.lines_f.resize(set[i].second);
            std::copy(p, p + set[i].second, tmp.lines_f.begin());
            p += set[i].second;
            tmp.write_cgats(set[i].first + file_tail);
            if (make_argyll)
            {
                if (fp != nullptr && !(i == 0 && set[i].first.length() >= 4 && set[i].first.substr(0, 4) == "rand"))
                {
                    string base = set[i].first + file_tail.substr(0, file_tail.length() - 4);   // remove ".txt"
                    string baseA = base + "A";
                    fprintf(fp, "SET FNAME=%s\n", base.c_str());
                    fprintf(fp, "SET PROFNAME=%s\n", baseA.c_str());
                    fprintf(fp, "txt2ti3 -v %%FNAME%%.txt %%FNAME%%\n");
                    fprintf(fp, "colprof -v -r .3 -qh -D %%PROFNAME%%.icm -O %%PROFNAME%%.icm %%FNAME%%\n\n");
                }
            }
        }
    }
    if (fp != nullptr) fclose(fp);
}

void CGATS_Bidir::print_random_patch_stats()
{
    auto v = m_fwd.getv_rgblab();
    auto vv = gather_patch_info(v);
    size_t extras = vv.pcount;
    size_t dup_cnt = vv.dup;
    if (extras * dup_cnt == 0)
        return;

    StatV3 lab_total;
    for (size_t patch = 0; patch < extras; patch++)
    {
        StatV3 lab;
        for (size_t cnt = 0; cnt < dup_cnt; cnt++)
            lab.clk(*(V3*)(&v[cnt + patch * dup_cnt][3]));
        lab_total.clk(lab);
    }

    printf("\nStatistics of Lab components from %zd extra patches duplicated %zd times\n", extras, dup_cnt);
    //printf("Ave std: L*=%5.2f, a*=%5.2f b*=%5.2f\n", lab_total[0].ave(), lab_total[1].ave(), lab_total[2].ave());
    printf("Ave std: L*=%5.2f, a*=%5.2f b*=%5.2f\n", V3(lab_total)[0], V3(lab_total)[1], V3(lab_total)[2]);
}



string get_profile_image(string profile_name)
{
    std::ifstream profilestream;
    profilestream.open(profile_name, std::ios::binary);
    if (profilestream.fail())
    {
        profile_name = string("C:\\Windows\\System32\\spool\\drivers\\color\\") + profile_name;
        profilestream.open(profile_name, std::ios::binary);
        if (profilestream.fail())
            throw "Profile file could not be found";
    }
    string profile_image;
    profilestream.seekg(0, std::ios_base::end);
    size_t size = (size_t)profilestream.tellg();
    profilestream.seekg(0, std::ios_base::beg);
    profile_image.resize(size);
    profilestream.read(profile_image.data(), size);
    return profile_image;
}

