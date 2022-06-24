#define _CRT_SECURE_NO_WARNINGS

#include <set>
#include <cmath>
#include "checkcolors.h"
#include "cgats.h"
#include "i1isis.h"
#include "pair_eval.h"
#include "mainutils.h"

using namespace cgats_utilities;
using namespace std::string_literals;

void print_dE_distr(vector<float> dE1, vector<float> dE2)
{
    auto print1 = [](vector<float>& dE, string title, bool add_header)
    {
        std::sort(dE.begin(), dE.end());
        float ave = std::accumulate(dE.begin(), dE.end(), 0.0f) / float(dE.size());
        if (add_header)
        {
            printf("\n                                      dE distribution (percent less than)\n"
                     "                    Ave         10%%   20%%   30%%   40%%   50%%   60%%   70%%   80%%   90%%\n");
        }
        printf("%s  %4.2f      ", title.c_str(), ave);
        for (auto x : { 10,20,30,40,50,60,70,80,90 })
            printf("%6.2f", dE[static_cast<size_t>(x / 100.0 * dE.size())]);
        printf("\n");
    };
    print1(dE1, "  Delta E 1976:  ", true);
    if (dE2.size())
        print1(dE2, "  Delta E 2000:  ", false);
}


// scale a RGB vector
void scale(vector<RGB>& rgb, float factor)
{
    for (auto& x : rgb)
        for (auto& y : x)
            y *= factor;
}

std::vector<V3> CheckColors::get_slope()
{
    std::vector<V3> ret(page_labs_mean[0].size());
    for (size_t color = 0; color < page_labs_mean[0].size(); color++)
        for (size_t labi = 0; labi < 3; labi++)
        {
            float s = 0, si = 0;
            float cm = 0;
            for (size_t page = 0; page < page_labs_mean.size(); page++)
                cm += page_labs_mean[page][color][labi];
            cm /= page_labs_mean.size();

            for (size_t page = 0; page < page_labs_mean.size(); page++)
            {
                float ii = float(page) / (page_labs_mean.size()-1);
                s += (ii - .5f) * (page_labs_mean[page][color][labi] - cm);
                si += (ii - .5f) * (ii - .5f);
            }
            s /= si;
            ret[color][labi] = s;
        }
    return ret;
}



void CheckColors::print(string filename) {
    if (colors_per_page == 0)
        return;
    auto slope = get_slope();

    printf("\nFile:%s\nCheck Colors per Page: %2d\n", filename.c_str(), colors_per_page);
    printf("            RGB         Mean L*a*b*        StDev L*a*b*        Slope L*a*b*\n");
    for (int color = 0; color < colors10; color++)
    {
        printf("%s:  %3.0f %3.0f %3.0f  %5.1f %5.1f %5.1f   %5.2f %5.2f %5.2f   %5.2f %5.2f %5.2f\n", color_descr(color),
            colors_rgb[color][0], colors_rgb[color][1], colors_rgb[color][2],
            labs_mean[color][0], labs_mean[color][1], labs_mean[color][2],
            color_stats[color].std()[0], color_stats[color].std()[1], color_stats[color].std()[2],
            slope[color][0], slope[color][1], slope[color][2]
        );
    }


    printf("\n                ");
    for (int i = 0; i < pages; i++)
        printf(" Lab pg:%2d          ", i + 1);
    printf("\n");
    for (int color = 0; color < colors10; color++)
    {
        printf("%s  ", color_descr(color));
        for (int page = 0; page < pages; page++)
        {
            auto lab = page_labs_mean[page][color];
            printf("%8.1f %5.1f %5.1f", lab[0], lab[1], lab[2]);
        }
        printf("\n");
    }
    printf("\n           Difference of each page from global average\n");
    for (int color = 0; color < colors10; color++)
    {
        printf("%s  ", color_descr(color));
        //    colors_rgb[color][0], colors_rgb[color][1], colors_rgb[color][2],
        //    labs_mean[color][0], labs_mean[color][1], labs_mean[color][2]);
        for (int page = 0; page < pages; page++)
        {
            auto diff = page_labs_mean[page][color] - labs_mean[color];
            printf("%8.1f %5.1f %5.1f", diff[0], diff[1], diff[2]);
        }
        printf("\n");
    }

    if (pages >= 3)
    {
        printf("\n       Std Dev of each page if >=20 Check Colors / page\n");
        for (int color = 0; color < colors10; color++)
        {
            printf("%s  ", color_descr(color));
            if (colors_per_page >= 20)
                for (int page = 0; page < pages; page++)
                {
                    auto std = page_labs_std[page][color];
                    printf("%8.1f %5.1f %5.1f", std[0], std[1], std[2]);
                }
            printf("\n");
        }
    }

    if (colors_per_page >= 20)
    {
        printf("\nMean Std Dev per page: ");
        for (auto x : std_page)
            printf("%7.4f", x);
        printf("\n");
    }
}


// check color/page consistency, rgblab is optional absence causing filename to be read
CheckColors::CheckColors(string filename, vector<V6> rgblab, PatchHeader header)
{
    this->filename = filename;
    if (rgblab.size()==0)
        rgblab = cgats_utilities::read_cgats_rgblab(filename);
    // calculate average lab values and std dev on the 10 repeated colors at the
    // end of each page. Length of colors is greater than 10.
    auto page_average = [](const vector<V3>& lab)
    {
        array<StatV3,10> c10;
        for (int i = 0; i < int(lab.size()); i++)
        {
            c10[i % 10].clk(lab[i]);
        }
        vector<V3> labout(10);
        vector<V3> laboutmin(10);
        vector<V3> laboutmax(10);
        vector<V3> laboutstd(10);
        for (int i = 0; i < 10; i++)
        {
            labout[i] = c10[i];
            laboutmax[i] = c10[i].max();
            laboutmin[i] = c10[i].min();
            if (lab.size() >= 20)
                laboutstd[i] = c10[i].std();
        }
        return tuple<vector<V3>, vector<V3>, vector<V3>, vector<V3>>(labout, laboutmin, laboutmax, laboutstd);
    };

    auto [rgb1, lab1] = separate_rgb_lab(rgblab);
    rgb = rgb1; lab = lab1;
    pages = int(header.pages);
    colors_per_page = int(header.colors_per_page);
    if (pages <= 1)
        return;
    patches_per_page = int(rgb.size() / pages);

    page_rgb_raw.resize(pages); page_lab_raw.resize(pages);
    for (int i = 0; i < pages; i++)
        for (int ii = 0; ii < patches_per_page; ii++)
        {
            page_rgb_raw[i].push_back(rgb[i * patches_per_page + ii]);
            page_lab_raw[i].push_back(lab[i * patches_per_page + ii]);
        }
    colors10 = colors_per_page > 10 ? 10 : colors_per_page;
    labs_mean = vector<V3>(colors_per_page);
    page_labs_mean = vector<vector<V3>>(pages, vector<V3>(colors_per_page));
    page_labs_all = vector<vector<V3>>(pages, vector<V3>(colors_per_page));
    page_labs_std = vector<vector<V3>>(pages);
    colors_rgb = vector<V3>(colors_per_page);
    if (pages >= 2)
    {
        for (int color = 0; color < colors_per_page; color++)
        {
            for (int page = 0; page < pages; page++)
            {
                page_labs_all[page][color] = V3{ lab[page * patches_per_page + patches_per_page - color - 1] };
                labs_mean[color] = labs_mean[color] + page_labs_all[page][color];
                color_stats[color % 10].clk(page_labs_all[page][color]);
            }
            colors_rgb[color] = colors_rgb[color] + V3{ rgb[patches_per_page - color - 1] };
        }
        scale(labs_mean, 1.0f / pages);	// makes average
        //page_labs_all = page_labs_mean;
        if (colors_per_page >= 10)
        {
            labs_mean = std::get<0>(page_average(labs_mean));
            for (size_t i = 0; i < page_labs_mean.size(); i++)
            {
                auto tmp = page_average(page_labs_all[i]);
                page_labs_mean[i] = std::get<0>(tmp);
                page_labs_std[i] = std::get<3>(tmp);
            }
            colors_rgb.resize(10);
        }
        if (page_labs_all[0].size() >= 20)
        {
            array<StatV3, 10> color_across_pages;
            vector<array<StatV3, 10>> color_per_page(pages);
            for (int i = 0; i < int(page_labs_all[0].size()); i++)
                for (int ii = 0; ii < pages; ii++)
                    color_across_pages[i % 10].clk(page_labs_all[ii][i]);
            for (int ii = 0; ii < pages; ii++)
                for (int i = 0; i < int(page_labs_all[0].size()); i++)
                    color_per_page[ii][i % 10].clk(page_labs_all[ii][i]);
            std_page = vector<double>(pages);
            for (int i = 0; i < pages; i++)
            {
                Statistics page;
                for (auto& xstat : color_per_page[i])
                    for (int i = 0; i < 3; i++)
                        page.clk(xstat.std()[i]);
                std_page[i] = page.ave();
            }
        }
    }
}


// ptr to 6 char description  of color location
const char* CheckColors::color_descr(int color) const
{
    const char* c[10] = { "NeutH ", "NeutM1","NeutM0","NeutL ",
         "LMag  ","LYel  ", "LCyn  ", "Mag   ", "Yel   ", "Cyn   " };
    return c[color];
};


void print2(const CheckColors& col1, const CheckColors& col2)
{
    validate(col1.colors_per_page == col2.colors_per_page && col1.pages == col2.pages, "measurement files don't match");
    printf("\n\nFile1: %s\nFile2: %s\n\n", col1.filename.c_str(), col2.filename.c_str());
    printf("             RGB        File1 L*a*b*       File2 L*a*b*                Diff \n");
    for (int color = 0; color < col1.colors10; color++)
    {
        printf("%s:  %3.0f %3.0f %3.0f  %5.1f %5.1f %5.1f  %5.1f %5.1f %5.1f     %5.1f %5.1f %5.1f\n", col1.color_descr(color),
            col1.colors_rgb[color][0], col1.colors_rgb[color][1], col1.colors_rgb[color][2],
            col1.labs_mean[color][0], col1.labs_mean[color][1], col1.labs_mean[color][2],
            col2.labs_mean[color][0], col2.labs_mean[color][1], col2.labs_mean[color][2],
            col1.labs_mean[color][0] - col2.labs_mean[color][0],
            col1.labs_mean[color][1] - col2.labs_mean[color][1],
            col1.labs_mean[color][2] - col2.labs_mean[color][2]);
    }
}


void print_fwd_rev_stats(CGATS_Bidir& cgats, CheckColors *ref_colors)
{
    printf("\n\nForward/Reverse stats, 5 worst dEs\n");
    if (ref_colors != nullptr && ref_colors->pages > 1)
        printf("Patch#  dE76        RGB              Lab Forward               Lab Reverse         page  row  column\n");
    else
        printf("Patch#  dE76        RGB              Lab Forward               Lab Reverse\n");
    auto rgb = cgats.m_fwd.getv_rgb();
    for (size_t i = 0; i < 5; i++)
    {
        const auto& x = cgats.stats[i];
        auto ipch = x.index;
        if (ref_colors != nullptr && ref_colors->pages > 1)
        {
            size_t ipg{};
            size_t ipch_pg{};
            size_t irow{};
            size_t icol{};
            if (cgats.header.is_i1pro2)
            {
                irow = ipch % (rgb.size() / 29);
                ipg = irow / 21;
                irow = irow % 21;
                icol = ipch / ((rgb.size() / 29));
            }
            else
            {
                ipg = ipch / ref_colors->patches_per_page;
                ipch_pg = ipch - ipg * ref_colors->patches_per_page;
                irow = ipch_pg % (ref_colors->patches_per_page / 29);
                icol = (ipch_pg - irow) / (ref_colors->patches_per_page / 29);
            }
            printf("%4zd   %5.2f    %3.0f %3.0f %3.0f   %7.2f %7.2f %7.2f    %7.2f %7.2f %7.2f   %2zd     %2zd    %2zd\n",
                1 + ipch,
                x.dE,
                x.rgb_f[0], x.rgb_f[1], x.rgb_f[2],
                x.lab_f[0], x.lab_f[1], x.lab_f[2],
                x.lab_r[0], x.lab_r[1], x.lab_r[2],
                1 + ipg,
                1 + irow,
                1 + icol
            );
        }
        else  // no page info available
        {
            printf("%4zd   %5.2f    %3.0f %3.0f %3.0f   %7.2f %7.2f %7.2f    %7.2f %7.2f %7.2f\n",
                1 + ipch,
                x.dE,
                x.rgb_f[0], x.rgb_f[1], x.rgb_f[2],
                x.lab_f[0], x.lab_f[1], x.lab_f[2],
                x.lab_r[0], x.lab_r[1], x.lab_r[2]
            );
        }
    }
    printf("\n             dE76 distribution (less than)\n"
        "   10%%   20%%   30%%   40%%   50%%   60%%   70%%   80%%   90%%\n");
    for (auto x : { 10,20,30,40,50,60,70,80,90 })
    {
        printf("%6.2f", cgats.stats[static_cast<size_t>((100.0 - x) / 100.0 * rgb.size())].dE);
    }
    printf("\n");
}

// create header, multiple of 3 bytes saved in array of floats from [0:255]
//   3 bytes reserved for adding rows and user patches
//   number of files: 1 byte
//   Repeat these two for each entry
//      RGB File Names Head: (variable, each name null terminated)
//      Length of RGB file, exclduing check colors in bytes: 2
//   null fill to multiple of 3 bytes
vector<V3> make_header(vector<pair<string, vector<V3>>> rgb_set)
{
    // build name, length
    string s{ "\0\0\0"s };
    s = s+ string{ static_cast<char>(rgb_set.size()) };  // Number of embedded files
    for (size_t i = 0; i < rgb_set.size(); i++)
    {
        s = s + rgb_set[i].first + "\0"s;
    }
    for (size_t i = 0; i < rgb_set.size(); i++)
    {
        auto len = rgb_set[i].second.size();
        s = s + static_cast<char>(len%256);
        s = s + static_cast<char>((len>>8) % 256);
    }
    while (s.length() % 3 != 0)
        s = s + "\0"s;
    auto x = s.length();
    vector<V3> ret(x/3);
    for (int i = 0; i < x; i++)
        ret[i/3][i%3] = *(unsigned char*)(s.data()+i);
    return ret;
}


// Must be called with "-e"
vector<V3> generate_rand_vecs(int argc, char** argv, int &index)
{
    // return vector of all ints in argv[index] and later
    auto num_args = [](int argc, char** argv, int& index, string &profile_image){
        vector<int> ret;
        for (int i=0; index+i < argc; i++)
        {
            int val;
            try {
                val = std::stoi(argv[index+i]);
                ret.push_back(val);
            }
            catch ([[maybe_unused]] std::exception& e) {
                if (is_suffix_icm(argv[index + 1]))
                    profile_image = get_profile_image(argv[index++ + 1]);
                break;
            }
        }
        index += int(ret.size());
        return ret;
    };
    string profile_image;
    bool all_random = false;
    auto nums = num_args(argc, argv, index, profile_image);
    int extras = nums.size() < 1 ? 250 : nums[0];
    int dup_cnt = nums.size() < 2 ? 1 : nums[1];
    unsigned seed = nums.size() < 3 ? 0 : nums[2];
    if (extras < 0)
    {
        extras = -extras;
        all_random = true;
    }
    else
        seed = 0;           // Force seed=0 for standard  multi-gamut

    validate(dup_cnt > 0, "-e option requires duplicatea count must be 1 or more");
    validate(extras * dup_cnt <= 5053, "Total number of extra patches must be less than 5053");

    auto sat = [](const V3& rgb) {
        float min_rgb = std::min(std::min(rgb[0], rgb[1]), rgb[2]);
        float max_rgb = std::max(std::max(rgb[0], rgb[1]), rgb[2]);
        return (max_rgb - min_rgb) / (min_rgb + 1000);
    };
    vector<V3> neutral_rgbs;
    int extras_neutral{}, extras_wide, extras_low;
    if (!all_random && extras >= 250)  // add 52 neutral axis RGBs
    {
        for (float i = 0; i <= 255; i += 5)
            neutral_rgbs.push_back({ i, i, i });
        extras_neutral = int(neutral_rgbs.size());
        auto tmp = extras - extras_neutral;
        extras_wide = int(.5f * tmp);
        extras_low = tmp - extras_wide;
    }
    else if (!all_random && extras > 100)  // add 18 neutral axis RGBs
    {
        for (float i = 0; i <= 255; i += 15)
            neutral_rgbs.push_back({ i, i, i });
        extras_neutral = int(neutral_rgbs.size());
        auto tmp = extras - extras_neutral;
        extras_wide = int(.5f * tmp);
        extras_low = tmp - extras_wide;
    }
    else
    {
        extras_wide = extras;
        extras_low = 0;
    }
    Distr_RGB randX;
    randX.profile_image = profile_image;
    vector<V3> all;

    // Convert device neturals to Rel Col neutrals
    if (!profile_image.empty())
    {
        auto rgb_to_lab = [&profile_image](const vector<V3>& rgb) {
            vector<V3> lab(rgb.size());
            convert_device_rgb_to_lab_mem(profile_image.data(), (int)profile_image.length(), (float(*)[3])rgb.data(), (float(*)[3])lab.data(), int(rgb.size()), 1);
            return lab;
        };
        auto lab_to_rgb = [&profile_image](const vector<V3>& lab) {
            vector<V3> rgb(lab.size());
            convert_lab_to_device_rgb_mem(profile_image.data(), (int)profile_image.length(), (float(*)[3])lab.data(), (float(*)[3])rgb.data(), int(lab.size()), 1);
            return rgb;
        };
        vector<V3> rgb{ V3{0,0,0}, V3{255,255,255} };
        vector<V3> lab_ends=rgb_to_lab(rgb);
        vector<V3> lab(neutral_rgbs.size());
        for (int i = 0; i < int(neutral_rgbs.size()); i++)
            lab[i][0] = lab_ends[0][0] + (lab_ends[1][0] - lab_ends[0][0]) * (i + .5f) / float(neutral_rgbs.size());
        neutral_rgbs = lab_to_rgb(lab);
        for (auto& x : neutral_rgbs)
            x[0] = round(x[0]), x[1] = round(x[1]), x[2] = round(x[2]);
    }

    for (auto& x : neutral_rgbs)
        all.push_back(x);

    if (extras_low > 0)
    {
        // generate a large number of randomly distributed points and select lowest saturation
        randX.make_random_separated_rgb(10 * extras_low, 5 * extras_low, seed);    // RGB low sat gamut distributed
        std::sort(randX.unique_rgb.begin(), randX.unique_rgb.end(), [&sat](auto& a, auto& b) {return sat(a) < sat(b); });
        randX.unique_rgb.resize(extras_low);
        for (auto& x : randX.unique_rgb)
            all.push_back(x);
    }
    {
        // make sure this is seeded differently by adding 1 to initial start count
        randX.make_random_separated_rgb((extras_low > 0 ? 10 : 2) * extras_wide + 1, extras_wide, seed);      // RGB gamut distributed
        for (auto& x : randX.unique_rgb)
            all.push_back(x);
    }
    // duplicate dup_cnt times
    vector<V3> rand_dup;
    for (auto& x : all)
        for (int i = 0; i < dup_cnt; i++)
            rand_dup.push_back(x);
    return rand_dup;
}


// ProfList - Create combined patches, "-T" options
ProfList::ProfList(int argc, char** argv)
{
    argv++; argc--;
    bool is_i1isis_tifs = "-T"s == argv[0];
    bool is_i1Pro2_tifs = "-T2"s == argv[0];
    bool is_i1isis_structure = is_i1isis_tifs || "-t"s == argv[0];
    bool is_i1Pro2_structure = is_i1Pro2_tifs || "-t2"s == argv[0];
    argv++; argc--;
    rows_per_page = std::stoi(argv[0]);
    {
        auto rows = std::abs(rows_per_page);
        if (is_i1isis_tifs)
            validate(rows >= 21 && rows <= 33, "i1isis charts must have between 21 and 33 rows");
        else if (is_i1Pro2_tifs)
            validate(rows == 21, "i1Pro2 charts must have exactly 21 rows");
    }

    // Special mode. Just create forward/reverse CGATs and associated tif files
    // useful for standard profiling charts
    if (rows_per_page < 0)
    {
        validate(argc == 2, "Only one file is allowed with -row option");
        rows_per_page = -rows_per_page;
        validate(is_suffix_txt(argv[1]), "Must be \".txt\" file");
        auto label = remove_suffix(argv[1])+"_x";
        vector<V3> rgb = read_cgats_rgb(argv[1]);
        if (rgb.size() % (29 * rows_per_page) != 0)
        {
            int residual = rgb.size() % (29 * rows_per_page);
            if (residual != 0)
            {
                printf("Adding %d extra RGB (255,255,255)/(200,200,200) patches for full chart size\n", 29 * rows_per_page - residual);
                for (int i = 0; i < 29 * rows_per_page - residual; i++)
                    if (i%2 == 0)
                        rgb.push_back({ 255,255,255 });
                    else
                        rgb.push_back({ 200,200,200 });
            }
        }
        int pages = int(rgb.size()) / (29 * rows_per_page);
        if (is_i1isis_tifs) {
            printf("Creating Tifs from %s.  %d Tifs with %d rows\n", argv[1], pages, rows_per_page);
            make_isis_txf(rgb, label, pages);
            make_i1isis(rgb, label, pages);
        }
        else if (is_i1Pro2_tifs)
        {
            make_i1pro2(rgb, label, pages);
            make_i1pro2_txf(rgb, label, pages);
        }
        cgats_utilities::write_cgats_rgb(rgb, label + ".txt");
        // Write a reverse RGB list. Can be used to read in tif files in reverse
        // useful to check consistency of patch reading. Good at picking up transient dust caused errors
        vector<RGB> rev(rgb.rbegin(), rgb.rend());
        cgats_utilities::write_cgats_rgb(rev, label + "r.txt");

        if (is_i1isis_tifs)
            make_isis_txf(rev, label+"r", pages);
        else if (is_i1Pro2_tifs)
            make_i1pro2_txf(rev, label + "r", pages);
        return;
    }

    validate(21 <= rows_per_page && rows_per_page <= 33, "Rows must be between 21 and 33");
    argv++; argc--;
    for (int i = 0; i < argc;)
    {
        if (argv[i]=="-e"s || argv[i] == "-E"s)
        {
            i++;
            auto rpatches = generate_rand_vecs(argc, argv, i);  // returns rand patches if option
            pair<string, vector<V3>> set;
            set.first = "rand"s + std::to_string(rpatches.size());
            set.second = rpatches;
            rgb_set.push_back(set);
        }
        else
        {
            pair<string, vector<V3>> set;
            validate(is_suffix_txt(argv[i]), "Must be \".txt\" file");
            set.first = remove_suffix(argv[i]);
            validate(set.first.length() <= 30, "RGB file base name must be <= 30 chars in length");
            set.second = read_cgats_rgb(argv[i]);
            for (auto& x : set.second)          // if RGB values fractional, (truncate - i1Profiler compatibility)
                for (auto& xx : x)
                    if (!global_modifiers::fractional)
                        xx = floor(xx);
            rgb_set.push_back(set);
            i++;
        }
    }
    // header encodes data on included files in initial rgb values to simplify reconstruction
    auto header = make_header(rgb_set);
    vector<V3> rgb_all;
    for (auto& x : rgb_set)
        rgb_all.insert(rgb_all.end(), x.second.begin(), x.second.end());

    rgb_all = randomize(rgb_all);   // randomize with seed=1
    rgb_all.insert(rgb_all.begin(), header.begin(), header.end());
    rgb_all[0]=encode_rows_and_patch_count(rows_per_page, int(rgb_all.size()), is_i1Pro2_structure);

    size_t pages_req = 1 + (rgb_all.size()-1) / (rows_per_page * 29 - 10);
    size_t check_color_count = pages_req * rows_per_page * 29 - rgb_all.size();
    size_t colors_per_allpages = check_color_count / pages_req;
    size_t check_color_residual = check_color_count - colors_per_allpages * pages_req;

    printf("Randomizing and adding edge check colors\n"
        "%zd pages of %d rows with 29 columns\n"
        "%zd check colors are added to the right edge of each page\n"
        "Randomized patch count=%zd\n"
        "Total patch count=%zd\n"
        "%zd patches can be added before additional pages are created\n",
        pages_req, rows_per_page, colors_per_allpages, rgb_all.size()-header.size(), rows_per_page*pages_req*29,
        check_color_residual + (colors_per_allpages - 20) * pages_req);

    auto p = rgb_all.begin();
    size_t patches_per_page = 29 * rows_per_page;
    vector<V3> rgb_out(patches_per_page* pages_req);
    for (size_t page = 0; page < pages_req; page++)
    {
        size_t color_check_start = patches_per_page - colors_per_allpages;
        if (check_color_residual != 0)
        {
            color_check_start--;
            check_color_residual--;
        }
        for (size_t i = 0; i < patches_per_page; i++)
            if (i < color_check_start)
            {
                rgb_out[page * patches_per_page + i] = *p++;
                auto zz = p - rgb_all.begin();
            }
            else
            {
                rgb_out[page * patches_per_page + i] = edge_colors[(i + (10 - patches_per_page % 10)) % 10];
                auto zz = p - rgb_all.begin();
            }
    }
    string label = "";
    for (auto& x : rgb_set)
        label += x.first + "__"s;
    label = "P_"s + label.substr(0, label.length() - 2);
    validate(p - rgb_all.begin() == rgb_all.size(), "Bug: p - rgb_all.begin() == rgb_all.size()");
    printf("Creating single, grouped CGATs and Tif image files: %s\n", label.c_str());

    if (is_i1Pro2_structure)
    {
        rgb_out = change_i1_order(rgb_out, 21, 29, true);
        printf("i1Pro2 layout structure.\n");
    }

    // Fwd RGB CGATs RGB patch list
    cgats_utilities::write_cgats_rgb(rgb_out, label + ".txt");
    vector<RGB> rev(rgb_out.rbegin(), rgb_out.rend());
    // Rev RGB CGATs RGB patch list
    cgats_utilities::write_cgats_rgb(rev, label + "r.txt");

    if (is_i1isis_structure)
    {
        make_isis_txf(rgb_out, label, int(pages_req));
        make_isis_txf(rev, label + "r", int(pages_req));
    }
    if (is_i1Pro2_structure)
    {
        make_i1pro2_txf(rgb_out, label, int(pages_req));
        make_i1pro2_txf(rev, label + "r", int(pages_req));
    }
    if (is_i1isis_tifs)
    {
        make_i1isis(rgb_out, label, int(pages_req));
    }
    if (is_i1Pro2_tifs)
    {
        make_i1pro2(rgb_out, label, int(pages_req));
    }
}

// determine if patch set is duplicated exactly N times, if so
// return patches and duplicate count
PatchSet gather_patch_info(const vector<V6>& rgblab)
{
    PatchSet ret;
    std::set<V3> urgb;
    for (const auto& x : rgblab)
    {
        urgb.insert(*(V3*)&x);
    }
    ret.dup = int(size(rgblab) / size(urgb));
    ret.pcount = int(size(rgblab) / ret.dup);

    if (size(rgblab) % size(urgb) == 0 && ret.dup > 1)
    {
        // All duplicates must be sequential, ie:  1,1,5,5,3,3,....
        for (int i = 0; i < ret.pcount; i++)
            for (int ii = 1; ii < ret.dup; ii++)
                if (*(V3*)&rgblab[i * ret.dup] != *(V3*)&rgblab[i * ret.dup + ii])
                {
                    ret.pcount = (int)size(rgblab);
                    ret.dup = 1;
                    return ret;
                }
    }
    // if size not a multiple of dup, revert
    else
    {
        ret.pcount = (int)size(rgblab);
        ret.dup = 1;
    }
    return ret;
}

// search for close lab value that matches rgb and return optimized lab vals
pair<vector<V3>,vector<float>> refine_lab(const vector<V3>& rgb, string prof, bool rev_only)
{
    auto rgb_to_lab = [&prof](const vector<V3>& rgb) {
        vector<V3> lab(rgb.size());
        convert_device_rgb_to_lab_mem(prof.data(), (int)prof.length(), (float(*)[3])rgb.data(), (float(*)[3])lab.data(), int(rgb.size()), 3);
        return lab;
    };
    auto lab_to_rgb = [&prof](const vector<V3>& lab) {
        vector<V3> rgb(lab.size());
        convert_lab_to_device_rgb_mem(prof.data(), (int)prof.length(), (float(*)[3])lab.data(), (float(*)[3])rgb.data(), int(lab.size()), 3);
        return rgb;
    };
    auto dup125 = [](const vector<V3>& v) {
        vector<V3> ret; ret.reserve(125 * v.size());
        for (auto x : v)
        {
            vector<V3> e(125, x);
            ret.insert(ret.end(), e.begin(), e.end());
        }
        return ret;
    };

    auto lab_opt_pass = [&lab_to_rgb, &dup125](const vector<V3>& lab, const vector<V3>& rgb, float scale)
    {
        vector<V3> lab_ret = lab;
        vector<float> rgb_err(lab.size());
        vector<V3> lab0; lab0.reserve(125 * lab.size());
        for (auto x : lab)
        {
            // duplicate each entry 125 times sequentially
            vector<V3> e(125, x);
            lab0.insert(lab0.end(), e.begin(), e.end());
        }
        // make a scale adjustment matrix
        vector<V3> s; s.reserve(125);
        for (int i = -2; i <= 2; i++)
            for (int ii = -2; ii <= 2; ii++)
                for (int iii = -2; iii <= 2; iii++)
                    s.push_back({ scale * i, scale * ii, scale * iii });
        for (int i = 0; i < 125 * rgb.size(); i++)
            lab0[i] = lab0[i] + s[i % 125];
        auto rgb0 = lab_to_rgb(lab0);
        auto rgba = dup125(rgb);
        auto err = deltaE(rgba, rgb0);
        for (size_t i = 0; i < lab.size(); i++)
        {
            auto p = std::min_element(err.begin() + i * 125, err.begin() + i * 125 + 125) - (err.begin() + i * 125);
            if (rgb[i][0] != 0 && rgb[i][1] != 0 && rgb[i][2] != 0 && rgb[i][0] != 255 && rgb[i][1] != 255 && rgb[i][2] != 255)
                lab_ret[i] = lab0[i * 125 + p];
            rgb_err[i]=err[i * 125 + p];
        }
        return pair{ lab_ret, rgb_err};
    };
    auto x = dup125(rgb);
    auto lab = rgb_to_lab(rgb);
    if (rev_only)
        return { lab, vector<float>(lab.size()) }; // skip refinement if reverse only lookup
    auto opt_results = lab_opt_pass(lab, rgb, 0);
    auto opt_results_init = opt_results;
    for (float i = 0, scale = .125f; i < 17; i++, scale *= .7f)
    {
        opt_results = lab_opt_pass(opt_results.first, rgb, scale);
        //cout << opt_results.second << endl;   // debug, show improvements per generation
    }
    // remove failures. Observed matches are typically .01-.02, with profile mapping errors > 4
    int count = 0;
    for (size_t i=0; i < opt_results.first.size(); i++)
        if (opt_results.second[i] > 1.0f)
        {
            opt_results.first[i] = opt_results_init.first[i];
            opt_results.second[i] = 99;
            count++;
        }
    return opt_results;
}