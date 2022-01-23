#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <algorithm>
#include "mainutils.h"
#include "deltae.h"
using namespace std::string_literals;

using std::tuple;


vector<Lab_dE_Set> get_color_info(vector<V3> laba, vector<V3> labb, string profname, int neutral_size)
{
    auto ave = [](vector<float> a) {return std::accumulate(a.begin(), a.end(), 0.0f) / a.size(); };
    auto mean_sat = [&ave](vector<V3> lab) {
        vector<float> v;
        for (auto& x : lab)
            v.push_back(sqrt(x[1] * x[1] + x[2] * x[2]));
        sort(v.begin(), v.end());
        return ave(v);
    };
    auto mean_std = [](vector<float> dE) {
        Statistics x;
        for (auto v : dE)
            x.clk(v);
        return x.std() / sqrt(float(x.n()));
    };

    auto eval_lab_grp = [&ave, &mean_sat, &mean_std](const vector<V3>& a, const vector<V3>& b) {
        Lab_dE_Set ret;
        vector<std::tuple<float, V3, V3>> v_laba_labb;       // dEs and lowest saturation lab1, lab2 except neutrals
        for (size_t i = 0; i < a.size(); i++)
        {
            ret.dE76.push_back(deltaE(a[i], b[i], 0));
            ret.dE2000.push_back(deltaE(a[i], b[i], 1));
        }
        std::sort(ret.dE76.begin(), ret.dE76.end());
        std::sort(ret.dE2000.begin(), ret.dE2000.end());
        ret.mean_dE_76 = ave(ret.dE76);
        ret.mean_std_dE_76 = mean_std(ret.dE76);
        ret.mean_dE_2000 = ave(ret.dE2000);
        ret.mean_std_dE_2000 = mean_std(ret.dE2000);
        ret.mean_saturation = mean_sat(a);
        return ret;
    };

    vector<Lab_dE_Set> ret;
    vector<V3> laba_n, labb_n;
    if (neutral_size)
    {
        laba_n.insert(laba_n.end(), laba.begin(), laba.begin() + neutral_size);
        labb_n.insert(labb_n.end(), labb.begin(), labb.begin() + neutral_size);
        laba.erase(laba.begin(), laba.begin() + neutral_size);
        labb.erase(labb.begin(), labb.begin() + neutral_size);
    }
    vector<tuple<float, V3, V3>> v_low_sat_laba_labb;       // dEs and lowest saturation lab1, lab2 except neutrals
    vector<tuple<float, V3, V3>> v_full_gamut_laba_labb;       // dEs and lowest saturation lab1, lab2 except neutrals
    // First half contains low saturation RGBs
    for (size_t i = 0; i < laba.size() / 2; i++)
        v_low_sat_laba_labb.push_back(tuple<float, V3, V3> {sqrt(laba[i][1] * labb[i][1] + laba[i][2] * labb[i][2]), laba[i], labb[i]});
    // Second half contains full gamut RGBs
    for (size_t i = laba.size() / 2; i < laba.size(); i++)
        v_full_gamut_laba_labb.push_back(tuple<float, V3, V3> {sqrt(laba[i][1] * labb[i][1] + laba[i][2] * labb[i][2]), laba[i], labb[i]});
    if (laba_n.size() != 0)
    {
        vector<V3> laba, labb;
        ret.push_back(eval_lab_grp(laba_n, labb_n));
        {
            Lab_dE_Set zzz;
            vector<V3> laba, labb;
            for (auto& x : v_low_sat_laba_labb)
            {
                laba.push_back(std::get<1>(x));
                labb.push_back(std::get<2>(x));
            }
            ret.push_back(eval_lab_grp(laba, labb));
        }
        {
            vector<V3> laba, labb;
            for (auto& x : v_full_gamut_laba_labb)
            {
                laba.push_back(std::get<1>(x));
                labb.push_back(std::get<2>(x));
            }
            ret.push_back(eval_lab_grp(laba, labb));
        }
    }
    else    // if no neutrals, only do one saturation
    {
        vector<V3> laba, labb;
        for (auto& x : v_low_sat_laba_labb)
        {
            laba.push_back(std::get<1>(x));
            labb.push_back(std::get<2>(x));
        }
        for (auto& x : v_full_gamut_laba_labb)
        {
            laba.push_back(std::get<1>(x));
            labb.push_back(std::get<2>(x));
        }
        ret.push_back(eval_lab_grp(laba, labb));
    }
    return ret;
}

// Neutral tests sequential L* increases
// Check seq for 52 or 18 neutrals or close neutrals and validate end point.
int get_size_of_neutrals(PatchCollection& vc)
{
    auto& v = vc.ave;   // vector of averaged colors. May be same as vc.full if not duplicated
    if (v.size() < 100) // Less than 100 means no neutral colors
        return 0;
    for (size_t i = 1; i < v.size(); i++)
        if (v[i - 1].labm[0] > v[i].labm[0] + 3)
        {
            if (i >= 18 && i < 52)
                return 18;
            else if (i >= 52 && i < 80) // 80 reduces odds to insignificant of random values being sequential increases
                return 52;
            else
                return 0;
        }
    return 0;
}


ColorAccuracy print_dE_stats(PatchCollection& collection, string prof_name, bool col_ave)
{
    auto ave = [](vector<float> a) {return std::accumulate(a.begin(), a.end(), 0.0f) / a.size(); };
    auto print_group = [](const Lab_dE_Set& color_set, string title) {
        printf("%s N=%zd, Ave Sat:%6.2f\n"
            "      Delta E 1976  Ave:%5.3f, Est. error of Ave:%4.3f\n"
            "      Delta E 2000  Ave:%5.3f, Est. error of Ave:%4.3f\n",
            title.c_str(),
            color_set.dE76.size(),
            color_set.mean_saturation,
            color_set.mean_dE_76, color_set.mean_std_dE_76,
            color_set.mean_dE_2000, color_set.mean_std_dE_2000);
        print_dE_distr(color_set.dE76, color_set.dE2000);
    };

    ColorAccuracy color_dE; color_dE.profile_name = prof_name;
    // select all patches ir averaged patches if RGBs are duplicated
    vector<RGB_LABM_LABI>& select = col_ave ? collection.ave : collection.full;
    string prof_image = get_profile_image(prof_name);
    auto neutral_size = get_size_of_neutrals(collection);

    if (neutral_size == 0)  // don't break down colors by saturation, analyze entire patch set
    {
        auto color_accuracy = get_color_info(get_labm(select), get_labi(select), "", neutral_size);
        print_group(color_accuracy[0], "\nAll patches");
        color_dE.dE76_full_gamut = color_accuracy[0].mean_dE_76;
        color_dE.dE2k_full_gamut = color_accuracy[0].mean_dE_2000;
    }
    else
    {
        auto color_accuracy = get_color_info(get_labm(select), get_labi(select), "", neutral_size * int(select.size() / collection.ave.size()));

        printf("\n---Using profile %s to analyze %zd patches, Dup Count: %zd---\n\n",
            prof_name.c_str(), select.size(), collection.full.size() / select.size());

        print_group(color_accuracy[0], "Neutrals");
        color_dE.dE76_dev_neutrals = color_accuracy[0].mean_dE_76;
        color_dE.dE2k_dev_neutrals = color_accuracy[0].mean_dE_2000;

        print_group(color_accuracy[1], "\n\n\nLow Saturation");
        color_dE.dE76_low_saturation = color_accuracy[1].mean_dE_76;
        color_dE.dE2k_low_saturation = color_accuracy[1].mean_dE_2000;

        print_group(color_accuracy[2], "\n\n\nFull Gamut");
        color_dE.dE76_full_gamut = color_accuracy[2].mean_dE_76;
        color_dE.dE2k_full_gamut = color_accuracy[2].mean_dE_2000;
    }
    printf("\n\n");
    return color_dE;
}

// returns vector of ranges to select one or more portions of a CGATs file.
// returns 0 length vector if no optional ranges
// Also adjusts argv, argc to the next potential CGATs file
vector<std::pair<int, int>> get_optional_range_v(size_t cgats_size, int& argc, char** &argv)
{
    vector<std::pair<int, int>> ret;
    auto check_range = [cgats_size](int v) {
        return (v < 1 || v > cgats_size) ? -1 : v;
    };
    while (argc >= 2)
    {
        int i1 = check_range(get_int(argv[0]));
        if (i1 == -1)              // normal exist if no more location pairs
            return ret;
        int i2 = check_range(get_int(argv[1]));
        if (i1 > 0 && i2 > 0)
        {
            ret.push_back({ i1, i2 });
            argc -= 2;
            argv += 2;
        }
        else
            throw std::runtime_error("Error in optional location pairs");
    }
    return ret;
}