#ifndef LCMS_WRAPPER
#define LCMS_WRAPPER

#include "string.h"

// Uncomment to remove LCMS dependency. LCMS provides the ability to create tracking profiles
// by creating patch sets that add near neutrals along actual neutral axis. Also used
// for additional patches near a set of Lab colors
#define HAS_LCMS

#ifdef HAS_LCMS
#include "lcms2.h"

// Test if profile exists in Windows OS
int is_profile_valid(const char* profile)
{
    if (strlen(profile) > 100)
        return 0;
    char full_path_profile[200] = "C:\\Windows\\System32\\spool\\drivers\\color\\";
    cmsHPROFILE hOutProfile;
    hOutProfile = cmsOpenProfileFromFile(profile, "r");
    if (hOutProfile == NULL)
    {
        strcat_s(full_path_profile, 200, profile);
        hOutProfile = cmsOpenProfileFromFile(full_path_profile, "r");
        if (hOutProfile == NULL)
            return 0;
    }
    cmsCloseProfile(hOutProfile);
    return 1;
}

//convert_lab_to_device_rgb_mem(prof_mem.data(), int(prof_mem.size()), (double(*)[3])lab_span.data(), (double(*)[3])rgb_span.data(), 125, 3);
int convert_lab_to_device_rgb_mem(const void* profilemem, int prof_len, const float lab[][3], float rgb[][3], int len, int colorimetric_mode)
{
    cmsHPROFILE hInProfile, hOutProfile;
    cmsHTRANSFORM hTransform;
    cmsCIELab Lab;
    double rgb_d[3];
    
    hOutProfile = cmsOpenProfileFromMem(profilemem, prof_len);
    hInProfile = cmsCreateLab4Profile(NULL);
    hTransform = cmsCreateTransform(hInProfile,
        TYPE_Lab_DBL,
        hOutProfile,
        TYPE_RGB_DBL,
        colorimetric_mode, 0);
    cmsCloseProfile(hInProfile);
    cmsCloseProfile(hOutProfile);
    for (int i = 0; i < len; i++)
    {
        // Fill in the Float Lab
        Lab.L = lab[i][0];
        Lab.a = lab[i][1];
        Lab.b = lab[i][2];
        cmsDoTransform(hTransform, &Lab, rgb_d, 1);
        rgb[i][0] = (float)(255 * rgb_d[0]);
        rgb[i][1] = (float)(255 * rgb_d[1]);
        rgb[i][2] = (float)(255 * rgb_d[2]);
    }
    cmsDeleteTransform(hTransform);
    return 1;
}

int convert_device_rgb_to_lab_mem(const void* profilemem, int prof_len, const float rgb[][3], float lab[][3], int len, int colorimetric_mode)
{
    cmsHPROFILE hInProfile, hOutProfile;
    cmsHTRANSFORM hTransform;
    //cmsCIELab Lab;
    double lab_d[3];
    hInProfile = cmsOpenProfileFromMem(profilemem, prof_len);
    hOutProfile = cmsCreateLab4Profile(NULL);
    hTransform = cmsCreateTransform(hInProfile,
        TYPE_RGB_DBL,
        hOutProfile,
        TYPE_Lab_DBL,
        colorimetric_mode, 0);
    cmsCloseProfile(hInProfile);
    cmsCloseProfile(hOutProfile);
    for (int i = 0; i < len; i++)
    {
        double rgbin[3];
        rgbin[0] = rgb[i][0] / 255;
        rgbin[1] = rgb[i][1] / 255;
        rgbin[2] = rgb[i][2] / 255;
        cmsDoTransform(hTransform, rgbin, lab_d, 1);
        lab[i][0] = (float)lab_d[0];
        lab[i][1] = (float)lab_d[1];
        lab[i][2] = (float)lab_d[2];
    }
    cmsDeleteTransform(hTransform);
    return 1;
}



#else
int is_profile_valid(const char* profile)
{
    return 0;
}
int convert_lab_to_device_rgb(const char* profile, const double lab[][3], double rgb[][3], int len, int colorimetric_mode)
{
    return 0;
}
#endif

#endif