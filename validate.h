#pragma once

// if not validated, throw with message
inline void validate(bool b, std::string message)
{
    if (!b)
        throw std::runtime_error(message.c_str());
}

inline bool is_suffix_txt(std::string fname)
{
    if (fname.size() <= 4)
        return false;
    return fname.substr(fname.size() - 4, 4) == ".txt";
}

inline bool is_cgats_measurement(std::string fname)
{
    if (fname.size() <= 7)
        return false;
    std::string end = fname.substr(fname.size() - 7, 7);
    return end == "_M0.txt" || end == "_M1.txt" || end == "_M2.txt" || end == "_M3.txt" ||
           end == "_m0.txt" || end == "_m1.txt" || end == "_m2.txt" || end == "_m3.txt";
}

inline bool is_cgats_rgb(std::string fname)
{
    return is_suffix_txt(fname) && !is_cgats_measurement(fname);
}

inline bool is_suffix_tif(std::string fname)
{
    if (fname.size() <= 4)
        return false;
    return fname.substr(fname.size() - 4, 4) == ".tif";
}

inline bool is_suffix_icm(std::string fname)
{
    if (fname.size() <= 4)
        return false;
    std::string end = fname.substr(fname.size() - 4, 4);
    return end == ".icc" || end == ".icm";
}

// return -1 if either value not numeric otherwise return string to int
inline int get_int(std::string s)
{
    for (size_t i = 0; i < s.size(); i++)
        if (!((s[i] >= '0' && s[i] <= '9') || s[i] == '+' || s[i] == '-'))
            return -1;
    return std::stoi(s);
}