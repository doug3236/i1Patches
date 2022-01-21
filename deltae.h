// get the CIE Delta E 2000 Color Difference betweewn Two Lab Values
#pragma once

template<class T>
T deltaE(const array<T, 3>& lab_std, const array<T, 3>& lab_sample, int dE2000 = 0)
{
    if (dE2000 == 0)
    {
        return sqrt(powf(lab_std[0] - lab_sample[0], 2) +
            powf(lab_std[1] - lab_sample[1], 2) +
            powf(lab_std[2] - lab_sample[2], 2));
    }
    const double pi = -2 * atan2(-1., 0.);
    double Lstd{ lab_std[0] }, astd{ lab_std[1] }, bstd{ lab_std[2] };
    double Lsample{ lab_sample[0] }, asample{ lab_sample[1] }, bsample{ lab_sample[2] };
    double cabarithmean = (sqrt(pow(astd, 2) + pow(bstd, 2)) + sqrt(pow(asample, 2) + pow(bsample, 2))) / 2.0f;
    double G = 0.5 * (1 - sqrt((pow(cabarithmean, 7) / (pow(cabarithmean, 7) + pow(25.f, 7)))));
    double apstd{ (1 + G) * astd };
    double apsample{ (1 + G) * asample };
    double Cpsample{ sqrt(pow(apsample,2) + pow(bsample, 2)) };
    double Cpstd{ sqrt(pow(apstd, 2) + pow(bstd, 2)) };
    double Cpprod = Cpsample * Cpstd;
    bool zcidx{ Cpprod == 0 };
    double hpstd = atan2(bstd, apstd);
    hpstd = hpstd + 2 * pi * (hpstd < 0);
    if (abs(apstd) + abs(bstd) == 0)
        hpstd = 0;
    double hpsample = atan2(bsample, apsample);
    hpsample = hpsample + 2 * pi * (hpsample < 0);
    if (abs(apsample) + abs(bsample) == 0)
        hpsample = 0;

    double dL = (Lsample - Lstd);
    double dC = (Cpsample - Cpstd);
    double dhp = (hpsample - hpstd);
    dhp = dhp - 2 * pi * (dhp > pi);
    dhp = dhp + 2 * pi * (dhp < (-pi));
    if (zcidx)
        dhp = 0;
    double dH = 2 * sqrt(Cpprod) * sin(dhp / 2);
    double Lp = (Lsample + Lstd) / 2;
    double Cp = (Cpstd + Cpsample) / 2;
    double hp = (hpstd + hpsample) / 2;
    hp = hp - (abs(hpstd - hpsample) > pi) * pi;
    hp = hp + (hp < 0) * 2 * pi;
    if (zcidx)
        hp = hpsample + hpstd;
    double Lpm502 = pow(Lp - 50, 2);
    double Sl = 1 + 0.015f * Lpm502 / sqrt(20 + Lpm502);
    double Sc = 1 + 0.045f * Cp;

    double Tx = 1 - 0.17f * cos(hp - pi / 6) + 0.24f * cos(2 * hp) + 0.32f * cos(3 * hp + pi / 30)
        - 0.20f * cos(4 * hp - 63 * pi / 180);

    double Sh = 1 + 0.015f * Cp * Tx;
    double delthetarad = (30 * pi / 180) * exp(-(pow((180 / pi * hp - 275) / 25, 2)));
    double Rc = 2 * sqrt((pow(Cp, 7)) / (pow(Cp, 7) + pow(25.f, 7)));
    double RT = -sin(2 * delthetarad) * Rc;

    const int kl = 1, kc = 1, kh = 1;
    double klSl = kl * Sl;
    double kcSc = kc * Sc;
    double khSh = kh * Sh;
    double de00 = sqrt(pow((dL / klSl), 2) + pow(dC / kcSc, 2) + pow(dH / khSh, 2) + RT * (dC / kcSc) * (dH / khSh));
    return (T)de00;
}

template<class T>
std::vector<T> deltaE(const std::vector<array<T, 3>>& lab_std, const std::vector<array<T, 3>>& lab_sample, int dE2000 = 0)
{
    validate(lab_std.size() == lab_sample.size(), "vectors must be same length");
    std::vector<T> ret(lab_sample.size());
    for (size_t i = 0; i < lab_std.size(); i++)
        ret[i] = deltaE(lab_std[i], lab_sample[i], dE2000);
    return ret;
}
