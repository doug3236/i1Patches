#include "pair_eval.h"



void Distr_RGB::initialize_rgb(size_t count, unsigned int seed)
{
    static std::mt19937 g0((unsigned int)count); // seed==0, don't re-initialize by making static
    std::mt19937 g(seed);   // if seed != 0 assume randomizer is re-initialized with seed
    std::uniform_int_distribution<> dist(0, 255);
    std::set<RGB> rgb;
    size_t new_count = count + rgb.size();
    while (rgb.size() < new_count)
        if (seed==0)
            rgb.insert(RGB{ float(dist(g0)), float(dist(g0)), float(dist(g0)) });
        else
            rgb.insert(RGB{ float(dist(g)), float(dist(g)), float(dist(g)) });
    rgb_list = vector<RGB>(rgb.begin(), rgb.end());
    lab_list = vector<RGB>(rgb_list.size());
    int convert_device_rgb_to_lab_mem(const void* profilemem, int prof_len, const float rgb[][3], float lab[][3], int len, int colorimetric_mode);
    if (profile_image.length() != 0)
        convert_device_rgb_to_lab_mem(profile_image.data(), (int)profile_image.length(), (float(*)[3])rgb_list.data(), (float(*)[3])lab_list.data(), int(rgb_list.size()), 3);
}

Summary Distr_RGB::make_random_separated_rgb(size_t nstart_i, size_t nend, unsigned int seed)
{
    auto dE = [](const RGB& lab1, const RGB& lab2) {
        auto square = [](float a, float b) {return (a - b) * (a - b); };
        return sqrt(square(lab1[0], lab2[0]) + square(lab1[1], lab2[1]) + square(lab1[2], lab2[2]));
    };
    do
    {
        // make "nstart" lists of random RGBs and associated Lab values
        initialize_rgb(nstart_i, seed);
        size_t nstart = rgb_list.size();

        // Make sets of all pssible combinations of two points (n-1)*n/2 not too far apart
        idx_de.clear();
        // use either rgb differences or Lab differences, if available, to determine separation of points
        vector<RGB>& rgb_or_lab_list = profile_image.length() == 0 ? rgb_list : lab_list;

        if (threshold == 0)
            for (uint16_t i = 0; i < rgb_or_lab_list.size() - 1; i++)
                for (uint16_t ii = i + 1; ii < rgb_or_lab_list.size(); ii++)
                    idx_de.push_back({ i, ii, dE(rgb_or_lab_list[i],rgb_or_lab_list[ii]) });
        else
            for (uint16_t i = 0; i < rgb_or_lab_list.size() - 1; i++)
                for (uint16_t ii = i + 1; ii < rgb_or_lab_list.size(); ii++)
                    if (float de = dE(rgb_or_lab_list[i], rgb_or_lab_list[ii]); de < threshold) // include only point pairs whre dE < threshold
                        idx_de.push_back({ i, ii, de });
        std::sort(idx_de.begin(), idx_de.end(), [](const IDX_De& a, const IDX_De& b) {return a.dE < b.dE; });

        // drop dE since vector is already sorted by dE
        // idx1 and idx2 are indexes into rgb_list and lab_list
        vector<array<uint16_t, 2>> seq; seq.reserve(idx_de.size());
        for (auto& x : idx_de)
            seq.push_back({ x.idx1, x.idx2 });
        size_t extras = rgb_list.size() - nend;
        size_t index = 0;
        std::unordered_set<uint16_t> u;

        int flip = 0;
        while (extras > 0 && index < seq.size())  // find and remove all extra points too close to other points
        {
            if (u.find(seq[index][flip ^ 1]) == u.end())   // if second point has not been excluded
            {
                //size_t n = u.size();
                if (u.find(seq[index][flip]) == u.end())
                {
                    u.insert(seq[index][flip]);    // make sure first point is excluded
                    extras--;
                }
                //if (n < u.size())
                flip ^= 1;
            }
            index++;
        }

        vector<array<uint16_t, 2>> possibles;
        // accumulate pairs that don't include excluded points
        for (auto const& x : seq)
            if (u.find(x[0]) == u.end() && u.find(x[1]) == u.end())
                possibles.push_back(x);

        vector<RGB2_dE> results;
        u.clear();
        for (const auto& x : possibles)
        {
            auto n = u.size();
            u.insert(x[0]);
            u.insert(x[1]);
            if (n != u.size())
                results.push_back({ rgb_list[x[0]], rgb_list[x[1]], dE(rgb_list[x[0]], rgb_list[x[1]]) });;
            if (u.size() == nend)
                break;
        }
        this->rgb2de = results;
        this->nend = nend;
        this->ntotal = int(rgb_list.size());
        unique_rgb.clear();
        for (auto x : u)
            unique_rgb.push_back(rgb_list[x]);
        if (u.size() != nend)
        {
            threshold *= 1.05f;
            continue;
        }
        else
        {
            Statistics stat_de;
            Summary ret;
            ret.rgb2de = rgb2de;
            ret.unique_rgb = unique_rgb;
            for (auto& x : rgb2de)
                stat_de.clk(x.dE);
            ret.dist_ave = stat_de.ave();
            ret.dist_min = stat_de.min();
            ret.dist_max = stat_de.max();
            ret.dist_std = stat_de.std();
            ret.dist_max_min = stat_de.max() - stat_de.min();
            return ret;
        }
    } while (true);
}

