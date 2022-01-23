#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <random>
#include <ctime>
#include <map>
#include <optional>
#include <filesystem>

#include "cgats.h"
#include "i1isis.h"
#include "checkcolors.h"
#include "cgats2.h"
#include "pair_eval.h"
#include "drift.h"
#include "mainutils.h"
#include "main.h"

using namespace std::string_literals;

inline char OptionFlag;

inline void set_option_flag(int argc, char** argv)
{
    if (argc >= 2)
    {
        OptionFlag = argv[1][1];
        if (OptionFlag > 'Z')
            OptionFlag -= 'a' - 'a';
    }
}

inline void print_option_info(char option)
{
    if (option >= 'a')  // make uppercase
        option = option + 'a' - 'A';
    const char *info;
    if (option == 'A')
    {
        info =
            "        i1Patches -A cgats_combined.txt cgats1.txt[start stop ...] cgats2.txt[start stop] ....\n"
            "            Combines and one or more CGATs files. Files can be RGB\n"
            "            only or measurement files but must all have the same layout.\n"
            "            Optionally, start and stop indexes can be specified for each cgats.For instance :\n"
            "            \"cgats1.txt 10 55\" will retrieve the 10'th through 55'th entries.\n"
            "            Multiple ranges can be given. \"cgats1.txt 1 5 7 10\" will retrieve\n"
            "            the first 9 values skipping the 6th\n"
            "            After combining the patches are randmized and the CGATs file is saved.\n"
            "\n"
            "        i1Patches -Ad cgats_combined.txt cgats1.txt cgats2.txt ....\n"
            "            Same as above but patches are de-randomized.\n"
            "\n"
            "        i1Patches -a cgats_combined.txt cgats1.txt cgats2.txt ....\n"
            "            Same as above but patches are not randomized.\n";
    }
    else if (option == 'C')
    {
        info =
            "        i1Patches - C test_patches_M2.txt profile1.icm profile2.icm ....\n"
            "            Evaluate profile(s) accuracy by determining the Lab values that generated a\n"
            "            corresponding RGB values and comparing them against the measured Lab values.\n"
            "            A high precision search is performed using BtoA profile tables.\n"
            "\n"
            "            i1Patches - c test_patches_M2.txt profile1.icm profile2.icm ....\n"
            "            Same as above but only AtoB tables in profiles are used.\n"
            "            This is faster but less accurate than the \"-C\" option since only BtoA tables are used.\n";
    }
    else if (option == 'R')
    {
        info =
            "        i1Patches -R measurement_file_M2.txt\n"
            "            Extract measurement files. Note that files can end in _M0.txt, _M1.txt of _M2.txt.\n"
            "            Measurement file should be in i1Profiler's CGATs format that includes,\n"
            "            at a minimum, RGBand Lab values.Recommend selecting all fields including\n"
            "            spectral data.For added data integrity, i1isis tif files can be scanned\n"
            "            in reverse from bottom to top and last to first page.If a CGATs measurement file\n"
            "            of the same name except ending in r_M2.txt instead of _M2.txt exists the\n"
            "            two files will be averaged and statistics printed on how closely the\n"
            "            patches match.This is useful for detecting transient issues such as lint on the print.\n"
            "\n"
            "            i1Patches -r measurement_file_M2.txt\n"
            "            Same as above but doesn't extract files. Just outputs statistics.\n"
            "\n"
            "            i1Patches -Ra measurement_file_M2.txt\n"
            "            Same as -R but also creates an Argyll batch file to make profiles.\n"
            "\n"
            "            i1Patches -Rx measurement_file_M2.txt\n"
            "            Only averages and saves the forward and reverse scans and prints stats.The averaged\n"
            "            file will be saved as *_ave_M2.txt. and can be any set of forward/reverse measurement files.\n"
            "            No structure, as produced with \"-T\" option, is required.\n";
    }
    else if (option == 'S')
    {
        info =
            "        i1patches -s file1.txt file2.txt\n"
            "            Compare two measurement files that have the same RGB values in the same sequence.\n"
            "            Useful to look at time variation in colors, paper/ink longevity\n";
    }
    else if (option == 'T')
    {
        info =
            "        i1Patches -[T|T2] n_rows[-e size] patch_set1.txt, patch_set2.txt, ...\n"
            "            Create RGB CGATs/txf files and tif image files. \"n_rows\" specifies the number of\n"
            "            rows on each page.It must be between 21 and 33 for i1isis, the \"-T\" option\n"
            "            and exactly 21 for the i1Pro2, the \"-T2\" option.\n"
            "            There are always 29 columns. Recommend 33 rows for full i1isis profile page size.\n"
            "            The optional \"-e size\" can be used to create a set of \"size\" RGB patches to compare\n"
            "            profile performance against. 500 or more will provide reasonable statistical\n"
            "            comparisons of accuracy between profiles. Zero or more \"-e\" options are allowed.\n"
            "            if no size argument is provided, a default of 250 is used.\n"
            "            if (size >= 250)      // add 52 neutral axis RGBs, split remaining among low saturation and full gamut.\n"
            "            else if (size > 100)  // add 18 deutral axis RGBs, split remaining among low saturation and full gamut.\n"
            "            else if (size <= 100) // Just add full gamut randomly spread RGB patches.\n"
            "\n"
            "            i1Patches -[T|T2] n_rows [-e size dup_count] patch_set1.txt, patch_set2.txt, ...\n"
            "            Same as above but all the \"-e\" patches generated are duplicated. This provides a means\n"
            "            of determining how much variation occurs when the same RGB patches printed\n"
            "            in different locations.\n"
            "\n"
            "            i1Patches -[T|T2] n_rows [-e size dup_count random_seed] patch_set1.txt, patch_set2.txt, ...\n"
            "            Same as above but seed can be specified with specific values used to create\n"
            "            the random patch set.This can be useful to check the variation that occurs\n"
            "            by creating multiple patch sets with or without the same RGB values.\n"
            "\n"
            "            i1Patches -[T|T2] -n_rows patch_set.txt\n"
            "            Note : n_rows is negative!. No coded structure or randomization applied.\n"
            "            This generates tif files and forward/reverse CGATs RGB files _x appended to\n"
            "            the base name. Useful for making tif profile targets that can be read both forward and\n"
            "            reverse with an i1isis, i1Pro 2 spectro. RGB CGATs file will be expanded\n"
            "            with white patches to be a multiple of 29*n_rows.\n"
            "            It produces charts the same as would appear in i1Profiler.\n"
            "\n"
            "            i1Patches -t ....\n"
            "            Same as \"-T\" above but only create RGB CGATs/txf files and\n"
            "            doesn't create tif images files. Use this when using\n"
            "            i1Profiler to print target images from the txf or RGB CGATs files.\n"
            "            In i1Profiler, use the default patches. 6mm for i1isis(Profile)\n"
            "            and 8mm(Landscape) for i1Pro2 on US letter size paper.\n";
    }
    else if (option == 'X')
    {
        info =
            "        i1Patches -X noise0 noise1 cgats_aggregate_rgb_file.txt iccProfile.icm\n"
            "            Generates pseudo forward/reverse measurement files using reference profile.\n"
            "            Useful for testingand provides patch quaility info that tracks printed\n"
            "            charts without printing.Provides initial guidance for selecting patch sets\n"
            "            to printand test.noise0and noise1 are the standard deveation of gaussian noise added\n"
            "            to synthesized Lab values from the RGB file.\n"
            "            noise0 should approximate the printer's patch to patch variation. noise1 should\n"
            "            approximate spectro instrument variation when reading the same patches.\n"
            "            I use noise0 = .2 and noise1 = .05\n";
    }
    else if (option == 'Z')
    {
        info =
            "        i1patches -Z rows file.txt\n"
            "            Create printer drift pattern and neutral ramp for testing color patch stability.\n"
            "            A RGB 4x4x4 plus 2:2:30 dev neutrals repeated to fill selected size\n"
            "            rows * 29 total patches.rows must be between 21 and 33 inclusive.\n"
            "            Created are an i1isis or i1Pro2 compatible file.tif and cgats/txf files, file.txt and filer.txt.\n"
            "\n"
            "        i1patches -Z  measurementfile1.txt [measurementfile2.txt]\n"
            "            Evaluate patch color consistency and L * ramp to adjust ink levels.\n"
            "            For two files, evaluate statistical differences between the two measurement files.This is\n"
            "            intended to test variation over time of 1 printed chart or comparing measurement\n"
            "            from 2 charts that were printed at different times, possibly with different ink levels\n"
            "            or after changing ink.\n";
    }
    else
    {
        info =
            "Summary of options. For detail, use \"i1patches -[T|R||C|A|Z|X|S]\" with the desired option.\n"
            " -T Create aggregated, randomized set with tif from one or more profile and verification patch sets.\n"
            " -R Disaggregate and extract measurement sets from CGATs measurement file.\n"
            " -C Compare ICC profiles against verification measurements.\n"
            " -A Create a CGATs set from one or more CGATs sets or portions thereof.\n"
            " -Z Create a pattern for testing color drift over time or between two separate prints.\n"
            " -X Synthesize a measurement file from CGATs RGB and an ICC Profile.\n"
            " -S Compare two measurement files with identical RGBs.\n";
            "\nACPU followed by any tif creating command: Expand tif images 2% for Adobe's ACPU.\n";
    }
    printf("%s\n\n", info);
}


int main(int argc, char** argv)
{
    char flag_char{};
    try {
        printf("Command: i1patches ");
        for (int i = 1; i < argc; i++)
            printf(" %s", argv[i]);
        printf("\n");

        // Hack to increase tif image size to partially accomodate Adobe's ACPU shrinkage
        extern bool expand_tif_2_percent;
        if (argc > 1 && "ACPU"s == argv[1])
        {
            expand_tif_2_percent = true;
            argv++; argc--;
        }

        if (argc <= 1)  // just list commands
            print_option_info(' ');
        else if (argc == 2)
        {
            print_option_info(argv[1][1]);
            exit(0);
        }
        else
            flag_char = argv[1][1];

        // -T: Create an RGB CGATs and txf files and associated i1isis compatible tiff files
        // -T2: Create an RGB CGATs and txf files and associated i1Pro2 compatible tiff files
        // -t: Create an RGB CGATs and txf files and print stats, no tif files
        if ("-t"s == argv[1] || "-T"s == argv[1] || "-T2"s == argv[1])
            // append CGATS targets and optional extra patches creating an aggregate RGB CGATS file
            // and create printable tif files
            ProfList profiles(argc, argv);

        // -R: Read measurement file, print stats and extract measurement files
        // -r: Read measurement file, print stats
        else if ("-r"s == argv[1] || "-R"s == argv[1] || "-Rx"s == argv[1] || "-Ra"s == argv[1])
            process_r(argc, argv);

        // Generate accuracy info for one or more profiles using a common set of
        // independant patches such as those generated by the "-T 33 -e 250 ..." command
        // Does a lab refined lookup (-C) or reverse only lookup (-c)
        // on each provided profile and print statistics for a common CGATs measurement file
        else if ("-c"s == argv[1] || "-C"s == argv[1])
            process_c(argc, argv);

        // create printer drift pattern and neutral ramp
        // RGB 4x4x4 plus 2:2:30 dev neutrals repeated to fill selected size
        // -Z n file.txt: creates n*29 patches in a tif image if -Z
        // as well as CGATs and txf files: file.txt and filer.txt for both -z and -Z
        else if ("-z"s == argv[1] || "-Z"s == argv[1])
            process_z(argc, argv);

        // aggregate files with option for a range within a files
        // -a:aggregate only, -A randomize after aggregation
        // -Ad:aggregate after de-randomization
        else if ("-a"s == string(argv[1],argv[1]+2) || "-A"s == string(argv[1],argv[1]+2))
            process_a(argv, argc);

        // compare two measurement files with identical RGB values
        else if ("-s"s == argv[1] || "-S"s == argv[1])
            process_s(argc, argv);

        // -X noise noise_fwd_rev rgb_file profile
        else if ("-x"s == argv[1] || "-X"s == argv[1])
            process_x(argc, argv);

        else
        {
            print_option_info(' ');
        }
        return 0;
    }
    catch ([[maybe_unused]] std::exception& e)
    {
        cout << "Exception: " << e.what() << std::endl << std::endl << std::endl;
        print_option_info(' ');
    }
    catch (...)
    {
        print_option_info(' ');
    }
}

/*
* -x generated.txt noise1 noise2 rgb_file.txt iccprofile.icm
* Use to synthesize RGBLAB "measurement files" using an
* RGB file and ICC profile. noise1 s/b around .2 and noise2 around .05
* These can be used to create profiles using smallr RGB sets to
* simulate accuracy of smaller charts. Mostly this was used for debugging.
*/
void process_x(int argc, char** argv)
{
    validate(argc == 6 && is_suffix_txt(argv[4]) && is_suffix_icm(argv[5]), "Bad arguments");
    float noise = std::stof(argv[2]) + .0001f;
    float noiser = std::stof(argv[3]) + .0001f;
    auto rgb = read_cgats_rgb(argv[4]);
    auto profile_image = get_profile_image(argv[5]);
    auto [lab, unused] = refine_lab(rgb, profile_image, true);
    string base = remove_suffix(argv[4]);
    vector<V6> rgblab, rgblabr;
    std::mt19937 g{ 1 };
    std::normal_distribution<> d{ 0, noise };
    std::normal_distribution<> dr{ 0, noiser };
    for (size_t i = 0; i < rgb.size(); i++)
        rgblab.push_back(V6{ rgb[i][0], rgb[i][1], rgb[i][2],
            lab[i][0] + float(d(g)),lab[i][1] + float(d(g)),lab[i][2] + float(d(g)) });
    // now create reverse CGATs with noise_fwd_rev added
    rgblabr = rgblab;
    for (size_t i = 0; i < rgb.size(); i++)
        for (size_t ii = 3; ii < 6; ii++)
            rgblabr[i][ii] += float(dr(g));
    std::reverse(rgblabr.begin(), rgblabr.end());
    printf("Synthesizing RGBLAB file from RGB file %s and profile %s\n", argv[4], argv[5]);
    write_cgats_rgblab(rgblab, (base + "_M2.txt").c_str());
    write_cgats_rgblab(rgblabr, (base + "r_M2.txt").c_str());
}

/*
* -s file1.txt file2.txt
* Compare two measurement files that have the same RGB values.
* Useful to lookat time variation in colors, paper/ink longevity
*/
void process_s(int argc, char** argv)
{
    validate(argc == 4, "Must have two identical measurement files");
    auto meas1 = CgatsMeasure(argv[2]);
    auto meas2 = CgatsMeasure(argv[3]);
    validate(meas1.getv_rgb().size() == meas2.getv_rgb().size(), "Must have two identical measurement files");
    vector<float> vec_dE;
    auto lab1 = meas1.getv_lab();
    auto lab2 = meas2.getv_lab();
    Statistics dE_stat, shift_L, shift_C;
    for (size_t i = 0; i < lab1.size(); i++)
    {
        auto saturation = [](V3 a) {return sqrt(a[1] * a[1] + a[2] * a[2]); };
        shift_L.clk(lab2[i][0] - lab1[i][0]);
        shift_C.clk(saturation(lab2[i]) - saturation(lab1[i]));
        dE_stat.clk(deltaE(lab1[i], lab2[i]));
        vec_dE.push_back(deltaE(lab1[i], lab2[i]));
    }
    printf("Average Overall Saturation change: %4.2f\n", shift_C.ave());
    printf("Average Overall Lightness (L*) change: %4.2f\n", shift_L.ave());
    print_dE_distr(vec_dE);
    printf("\n\n");
}


/*
*  -[A|Ad|a] aggregate.txt file1 [index1 index2] ... [one or more files and ranges
* Example: -A rgb_all.txt rgb1.txt rgb2.txt 1 10
*    Creates a randomized aggregate RGB file, rgb_all.txt, from all of rgb1.txt and the first 10 entries in rgb2.txt
* -A:  Aggregate from files and Randomize aggregate.
* -Ad: Aggregate from De-Randomized files
* -a:  Aggregate files, no randomization or de-randomization
*/
void process_a(char**& argv, int& argc)
{
    // argv* points to filename with optiona start/stop indexes after
    auto process_file = [](int& argc, char**& argv, bool de_randomize) {
        auto data = CgatsMeasure(argv[0]); argv++; argc--;
        auto pairs = get_optional_range_v(data.lines_f.size(), argc, argv);
        if (de_randomize) // de-randomize before trimming
            data.lines_f = randomize(data.lines_f, true);   // de-randomize
        if (pairs.size() == 0)
            return data;  // No subsets, return all CGATs data
        else
        {
            auto data_new = data;
            data_new.lines_f.resize(0);
            for (const auto& start_stop : pairs)
            {
                data_new.lines_f.insert(data_new.lines_f.end(),
                    data.lines_f.begin() + start_stop.first-1,
                    data.lines_f.begin() + start_stop.second);
            }
            return data_new;
        }
    };
    validate(argc >= 4, "must aggregate 1 or more cgats files");
    string target_file(argv[2]);

    CgatsMeasure accum;
    if ("-A"s == argv[1])      // "-A" aggregate files then randomize
    {
        argv += 3;
        argc -= 3;
        accum = process_file(argc, argv, false);
        while (argc >= 1)
        {
            auto x = process_file(argc, argv, false);
            validate(x.lines_f.size() > 0 && x.lines_f[0].size() == accum.lines_f[0].size(),
                "CGATs files do not have consistent data field lengths");
            accum.lines_f.insert(accum.lines_f.end(), x.lines_f.begin(), x.lines_f.end());
        }
        accum.lines_f = randomize(accum.lines_f);   // randomize with seed=1
    }
    else if ("-Ad"s == argv[1])      // "-A" aggregate from de-randomized files
    {
        argv += 3;
        argc -= 3;
        accum = process_file(argc, argv, true);
        while (argc >= 1)
        {
            auto x = process_file(argc, argv, true);
            validate(x.lines_f.size() > 0 && x.lines_f[0].size() == accum.lines_f[0].size(),
                "CGATs files do not have consistent data field lengths");
            accum.lines_f.insert(accum.lines_f.end(),x.lines_f.begin(), x.lines_f.end());
        }
    }
    else        // aggregate files
    {
        argv += 3;
        argc -= 3;
        accum = process_file(argc, argv, false);
        while (argc >= 1)
        {
            auto x = process_file(argc, argv, false);
            validate(x.lines_f.size() > 0 && x.lines_f[0].size() == accum.lines_f[0].size(),
                "CGATs files do not have consistent data field lengths");
            accum.lines_f.insert(accum.lines_f.end(), x.lines_f.begin(), x.lines_f.end());
        }
    }
    if (is_cgats_rgb(target_file))
        write_cgats_rgb(accum.getv_rgb(), target_file);
    else
        accum.write_cgats(target_file);
    printf("Created aggregate file  %s\n", target_file.c_str());
}

/*
* -[R|Rx|r] measurementfile.txt
* Prints stats and extract measurement files from aggregated measurement file from "-T" option
* File names are recovered automatically.
* 
* -r measurementfile.txt
* Just statistics are printed, no files extracted.
*/

void process_r(int argc, char** argv)
{
    validate(argc==3, "command not recognized");
    CGATS_Bidir cgats(argv[2], "-R"s == argv[1], "-Rx"s == argv[1], "-Ra"s == argv[1]);
    if (("-R"s == argv[1] || "-Rx"s == argv[1]) && cgats.has_forward_and_reverse)
        cgats.m_fwd.write_cgats(cgats.file_base + "_ave"s + cgats.file_tail);

    // only prints fwd/rev stats if not a valid measurement file from -T option
    if (cgats.has_forward_and_reverse && !cgats.header.header_valid)
        print_fwd_rev_stats(cgats);
    // process measurement file and optionally average fwd/rev files
    else if (cgats.header.header_valid)
    {
        CheckColors ref_colors(argv[2], cgats.m_fwd.getv_rgblab(), cgats.header);
        if (cgats.header.pages > 1)
            ref_colors.print(argv[2]);
        if (cgats.has_forward_and_reverse)
            print_fwd_rev_stats(cgats, &ref_colors);
    }
}

/*
* -Z nrows file.txt
* nrows must be between 21 and 33 inclusive. This creates a 4x4x4 RGB color grid
* and a device neutral ramp R=G=B(0,2,4,6,...30)
* These are randomized and repeated as required to fill nrows*29 patches.
* Used to optimize head gap, vacuum, etc. prior to profiling.
* 
* -Z measurementfile.txt read, evaluate and print statistics.
*/

void process_z(int argc, char** argv)
{
    argv++; argc--;
    bool create_tifs = "-Z"s == argv[0];
    argv++; argc--;
    size_t rows_per_page{};
    try { rows_per_page = std::stoi(argv[0]); }
    catch (...) {};

    // write CGATs drift RGB file(s)
    if (rows_per_page != 0)
    {
        validate(21 <= rows_per_page && rows_per_page <= 33, "Rows must be between 21 and 33");
        validate(is_suffix_txt(argv[1]), "Must be \".txt\" file");
        auto label = remove_suffix(argv[1]);
        
        vector<V3> rgb = ProcessDuplicates().get_page_rgb(rows_per_page * 29);
        cgats_utilities::write_cgats_rgb(rgb, label + ".txt");
        make_isis_txf(rgb, label, 1);
        vector<RGB> rev(rgb.rbegin(), rgb.rend());
        cgats_utilities::write_cgats_rgb(rev, label + "r.txt");
        make_isis_txf(rev, label + "r", 1);
        if (create_tifs)
            make_i1isis(rgb, label, 1);
    }
    else    // evaluate measurement file(s) if 0
    {
        CGATS_Bidir cgats1(argv[0], false, true);
        ProcessDuplicates drift1(cgats1.m_fwd.getv_rgblab());
        CGATS_Bidir cgats2;
        if (argc == 1)
        {
            print(drift1);
        }
        else
        {
            cgats2 = CGATS_Bidir(argv[1], false, true);
            ProcessDuplicates drift2(cgats2.m_fwd.getv_rgblab());
            print(drift1, drift2);
        }
    }
}


void process_c(int argc, char** argv)
{
    bool rev_only = "-c"s == argv[1];
    argc -= 2; argv += 2;
    auto remove_path_and_suffix = [](string file) {
        file = remove_suffix(file);
        return (std::filesystem::path(file).filename().generic_string());
    };
    vector<V6> rgblab = read_cgats_rgblab(argv[0]);
    PatchCollection rgb_lab_collection = make_rgb_labm_labi(rgblab);
    vector<ColorAccuracy> profs_summary;
    for (int i = 1; i < argc; i++)
    {
        //auto xxx = std::filesystem::path(argv[i]).filename();
        string prof = get_profile_image(argv[i]);
        update_labi_from_profile(rgb_lab_collection, prof, rev_only);
        string results = remove_path_and_suffix(argv[0]) + "__"s + remove_path_and_suffix(argv[i]) + ".icm.txt"s;
        printf("Creating Results File: %s\n", results.c_str());
        FILE* fp = fopen(results.c_str(), "wt");
        for (size_t i = 0; i < rgb_lab_collection.full.size(); i++)
        {
            auto& x = rgb_lab_collection.full;
            fprintf(fp, "%5.0f%5.0f%5.0f    %5.2f%7.2f%7.2f  %5.2f%7.2f%7.2f\n",
                x[i].rgb[0], x[i].rgb[1], x[i].rgb[2],
                x[i].labi[0], x[i].labi[1], x[i].labi[2],
                x[i].labm[0], x[i].labm[1], x[i].labm[2]);
        }
        fclose(fp);
        profs_summary.push_back(print_dE_stats(rgb_lab_collection, argv[i], false));
        if (rgb_lab_collection.ave.size() < rgb_lab_collection.full.size())
        {
            printf("\n\n\n-------------AVERAGED PATCHES-------------");
            print_dE_stats(rgb_lab_collection, argv[i], true);
        }
        printf("\n\n");
    }
    if (profs_summary[0].dE2k_dev_neutrals == 0)
        printf("All Patches\n dE76  dE2k\n");
    else
        printf("Device Neutrals  Low Saturation    Full Gamut\n"
            " dE76  dE2k       dE76  dE2k       dE76  dE2k\n");

    for (const auto& x : profs_summary)
    {
        if (x.dE2k_dev_neutrals == 0)
        {
            printf(" %4.2f  %4.2f   %s\n", x.dE76_full_gamut, x.dE2k_full_gamut, x.profile_name.c_str());
        }
        else
        {
            printf(" %4.2f  %4.2f       %4.2f  %4.2f       %4.2f  %4.2f       %s\n",
                x.dE76_dev_neutrals, x.dE2k_dev_neutrals,
                x.dE76_low_saturation, x.dE2k_low_saturation,
                x.dE76_full_gamut, x.dE2k_full_gamut,
                x.profile_name.c_str());
        }
    }
    printf("\n\n");
}
