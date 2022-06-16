#include <algorithm>
#include "drift.h"
#include "checkcolors.h"
#include <map>

ProcessDuplicates::ProcessDuplicates()
{   // Construct the standard "Drift" RGB set and assing it to defined_rgb
    vector<V3> rgbn;
    for (float i = 2; i <= 30; i += 2)
        rgbn.push_back(V3{ i, i, i });
    vector<V3> rgbc;
    for (float i = 0; i <= 255; i += 85)
        for (float ii = 0; ii <= 255; ii += 85)
            for (float iii = 0; iii <= 255; iii += 85)
                if (i == ii && i == iii)
                    if (i == 0)
                        rgbn.insert(rgbn.begin(), { 0,0,0 });
                    else
                        rgbn.insert(rgbn.end(), { i,i,i });
                else
                    rgbc.push_back(V3{ i, ii, iii });
    defined_rgb = rgbn;
    defined_rgb.insert(defined_rgb.end(), rgbc.begin(), rgbc.end());
}

vector<V6> ProcessDuplicates::get_both_averaged_rgblab() const
{
    vector<V6> ret;
    for (size_t i = 0; i < both_p.rgb.size(); i++)
        ret.push_back(V6{ both_p.rgb[i][0],both_p.rgb[i][1],both_p.rgb[i][2],
                          both_p.lab[i][0],both_p.lab[i][1],both_p.lab[i][2] });
    return ret;
}

ProcessDuplicates::ProcessDuplicates(vector<V6> in_rgblab) : ProcessDuplicates() 
{
    rgblab = in_rgblab;
    std::unordered_map<V3, StatV3, HashV3> colors;  // detected stats
    std::unordered_map<V3, StatV3, HashV3> neutrals;// detected stats
    std::unordered_map<V3, StatV3, HashV3> both;    // detected stats
    std::unordered_map<V3, Statistics, HashV3> dE;
    std::unordered_map<V3, Statistics, HashV3> dE2k;

    // Slow, but puts neutrals in sequential order
    for (int i = 0; i <= 255; i++)
        for (size_t pos = 0; pos < rgblab.size(); pos++)
        {
            auto loc = V3{ float(i),float(i),float(i) };
            auto rgb = *(V3*)&rgblab[pos];
            auto lab = *((V3*)&rgblab[pos] + 1);
            auto neutral_i = V3{ float(i),float(i),float(i) };
            if (neutral_i == rgb)
            {
                if (!neutrals.contains(loc))
                    neutrals_p.rgb.push_back(neutral_i);  // only record entry if the first one
                neutrals[neutral_i].clk(lab);
            }
        }
    for (size_t pos = 0; pos < rgblab.size(); pos++)
    {
        auto rgb = *(V3*)&rgblab[pos];
        auto lab = *((V3*)&rgblab[pos] + 1);
        if (std::find(defined_rgb.begin(), defined_rgb.end(), rgb) == defined_rgb.end())
            defined_rgb_valid = false;
        if (rgb[0] != rgb[1] || rgb[0] != rgb[2])
        {
            if (!colors.contains(rgb))
                colors_p.rgb.push_back(rgb);  // only record entry if the first one
            colors[rgb].clk(lab);
        }
    }
    both_p.rgb = neutrals_p.rgb;
    both_p.rgb.insert(both_p.rgb.end(), colors_p.rgb.begin(), colors_p.rgb.end());
    both = neutrals;
    for (auto& x : colors)
        both[x.first] = x.second;
    for (auto& x : both_p.rgb)
    {
        dE[x] = Statistics{};
        dE2k[x] = Statistics{};
    }
    for (auto& x : rgblab)
    {
        auto rgb = *(V3*)&x;
        auto lab = *((V3*)&x+ 1);
        auto lab_ave = both[rgb].operator V3();
        dE[rgb].clk(deltaE(lab, lab_ave));
        dE2k[rgb].clk(deltaE(lab, lab_ave, 1));
    }

    auto init = [](PatchGroup& x) {
        x.count.resize(x.rgb.size());
        x.dE.resize(x.rgb.size());
        x.dE2k.resize(x.rgb.size());
        x.lab.resize(x.rgb.size());
    };
        // initialize vector sizes
    init(neutrals_p);
    init(colors_p);
    init(both_p);
        
    for (size_t i = 0; i < both_p.rgb.size(); i++)
    {
        both_p.dE[i] = dE[both_p.rgb[i]].ave();
        both_p.dE2k[i] = dE2k[both_p.rgb[i]].ave();
        both_p.lab[i] = (V3)both[both_p.rgb[i]];
        both_p.count[i] = both[both_p.rgb[i]].n();
        if (i < neutrals_p.rgb.size())
        {
            neutrals_p.dE[i] = dE[both_p.rgb[i]].ave();
            neutrals_p.dE2k[i] = dE2k[both_p.rgb[i]].ave();
            neutrals_p.lab[i] = (V3)both[both_p.rgb[i]];
            neutrals_p.count[i] = both[both_p.rgb[i]].n();
        }
        else
        {
            size_t ii = i - neutrals_p.rgb.size();
            colors_p.dE[ii] = dE[both_p.rgb[i]].ave();
            colors_p.dE2k[ii] = dE2k[both_p.rgb[i]].ave();
            colors_p.lab[ii] = (V3)both[both_p.rgb[i]];
            colors_p.count[ii] = both[both_p.rgb[i]].n();
        }
    }
}

void print(const ProcessDuplicates& first, const ProcessDuplicates& second)
{
    auto common = [](vector<V3> v1, vector<V3> v2)
    {
        std::map<V3, size_t> v2m;
        for (size_t i = 0; i < v2.size(); i++)
            v2m[v2[i]] = i;
        vector<std::pair<size_t, size_t>> ret;
        for (size_t i = 0; i < v1.size(); i++)
            if (v2m.contains(v1[i]))
                ret.push_back({i,v2m[v1[i]] });
        return ret;
    };
    auto popdEs = [](const PatchGroup& pg, size_t loc) {
        if (pg.count[loc] > 1)
        {
            return std::pair(pg.dE[loc] / sqrtf(1.0f*(pg.count[loc]-1)), pg.dE2k[loc] / sqrtf(1.0f * (pg.count[loc]-1)));
        }
        else
        {
            return std::pair{ 0.0f, 0.0f };
        }
    };
    if (second.rgblab.size() == 0)
    {
        printf("Device Neutral Patches\n");
        for (size_t i = 0; i < first.neutrals_p.rgb.size(); i++)
            printf("RGB(%3d,%3d,%3d)  Lab=%5.2f %6.2f %6.2f   dE76:%5.2f, dE2k:%5.2f   Acc dE76:%5.2f, dE2k:%5.2f\n",
                int(first.neutrals_p.rgb[i][0]), int(first.neutrals_p.rgb[i][1]), int(first.neutrals_p.rgb[i][2]),
                first.neutrals_p.lab[i][0], first.neutrals_p.lab[i][1], first.neutrals_p.lab[i][2], first.neutrals_p.dE[i], first.neutrals_p.dE2k[i],
                popdEs(first.neutrals_p, i).first, popdEs(first.neutrals_p, i).second);
        printf("\nColor Patches\n");
        for (size_t i = 0; i < first.colors_p.rgb.size(); i++)
            printf("RGB(%3d,%3d,%3d)  Lab=%5.2f %6.2f %6.2f   dE76:%5.2f, dE2k:%5.2f   Acc dE76:%5.2f, dE2k:%5.2f\n",
                int(first.colors_p.rgb[i][0]), int(first.colors_p.rgb[i][1]), int(first.colors_p.rgb[i][2]),
                first.colors_p.lab[i][0], first.colors_p.lab[i][1], first.colors_p.lab[i][2], first.colors_p.dE[i], first.colors_p.dE2k[i],
                popdEs(first.colors_p, i).first, popdEs(first.colors_p, i).second);
        print_dE_distr(first.both_p.dE, first.both_p.dE2k);
        printf("\n\n");
    }
    else
    {
        auto common_both = common(first.both_p.rgb, second.both_p.rgb);
        auto common_neutrals = common(first.neutrals_p.rgb, second.neutrals_p.rgb);
        auto common_colors = common(first.colors_p.rgb, second.colors_p.rgb);
        vector<V3> rgb1, rgb2, lab1, lab2;

        auto common_groups = [](vector<pair<size_t, size_t>> sel, const PatchGroup& pg1, const PatchGroup& pg2)
        {
            PatchGroup grp1, grp2;
            auto one_grp = [](const vector<pair<size_t, size_t>>& sel, int set, const PatchGroup& grp)
            {
                PatchGroup ret;
                for (size_t i = 0; i < sel.size(); i++)
                {
                    size_t used;
                    used = set == 1 ? sel[i].first : sel[i].second;
                    ret.count.push_back(grp.count[used]);
                    ret.dE.push_back(grp.dE[used]);
                    ret.dE2k.push_back(grp.dE2k[used]);
                    ret.rgb.push_back(grp.rgb[used]);
                    ret.lab.push_back(grp.lab[used]);
                }
                return ret;
            };
            grp1 = one_grp(sel, 1, pg1);
            grp2 = one_grp(sel, 2, pg2);
            return pair{ grp1, grp2 };
        };

        auto xx = common_groups(common_both, first.both_p, second.both_p);

        //// Comparing two standard drift sets
        //if (first.defined_rgb_valid && second.defined_rgb_valid)
        //{
        auto print = [](const PatchGroup& p1, const PatchGroup& p2)
        {
            Statistics dE_ave, dE2k_ave;
            auto printline = [&](size_t i)
            {
                printf("RGB(%3d,%3d,%3d)  Lab1=%5.2f %6.2f %6.2f  Lab2=%5.2f %6.2f %6.2f   dE76=%5.2f, dE2k=%5.2f\n",
                    int(p1.rgb[i][0]), int(p1.rgb[i][1]), int(p1.rgb[i][2]),
                    p1.lab[i][0], p1.lab[i][1], p1.lab[i][2],
                    p2.lab[i][0], p2.lab[i][1], p2.lab[i][2],
                    deltaE(p1.lab[i], p2.lab[i]),
                    deltaE(p1.lab[i], p2.lab[i], 1));
                dE_ave.clk(deltaE(p1.lab[i], p2.lab[i]));
                dE2k_ave.clk(deltaE(p1.lab[i], p2.lab[i], 1));
            };
            struct dE_Errs {
                size_t i;
                float dE;
                float dE2k;
            };
            vector<dE_Errs> v(p1.rgb.size());
            vector<float> dE_v, dE2k_v;
            for (size_t i = 0; i < p1.rgb.size(); i++)
            {
                printline(i);
                v[i].i = i;
                dE_v.push_back(v[i].dE = deltaE(p1.lab[i], p2.lab[i]));
                dE2k_v.push_back(v[i].dE2k = deltaE(p1.lab[i], p2.lab[i], 1));
            }
            printf("dE76 Ave=%4.2f, dE2k=%4.2f\n\n", dE_ave.ave(), dE2k_ave.ave());
            std::sort(v.begin(), v.end(), [](dE_Errs& a, dE_Errs& b) {return a.dE + a.dE2k > b.dE + b.dE2k; });
            printf("Worst Case dEs\n");
            for (size_t i = 0; i < 5; i++)
                printline(v[i].i);
            print_dE_distr(dE_v, dE2k_v);
            printf("\n\n");
        };
        auto neutrals = common_groups(common_neutrals, first.both_p, second.both_p);
        printf("Device Neutral Patches\n");
        print(neutrals.first, neutrals.second);
        auto colors = common_groups(common_colors, first.both_p, second.both_p);
        printf("All Patches\n");
        print(colors.first, colors.second);
    }
}

