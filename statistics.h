#pragma once
#include <cmath>

#undef min
#undef max

//----------------------- Statistics ----------------
// Gather statistics on data
class Statistics {
private:
    int scount;
    float vmax;
    float vmin;
    double sum;
    double sum2;
public:
	void reset(){scount=0; sum=sum2=0.0f; vmax=-999.9f, vmin=999.9f;};	// clear accumulators
    Statistics(){reset();};
    void clk(float f);			// clock in data value
    float ave();				// get mean
    float stdp();				// get standard deviation, population
    float std();				// get standard deviation
    float min();				// get minimum
    float max();				// get maximum
	int n(){return scount;};
	Statistics operator+(Statistics &a);	// accumulate statistics
};

inline Statistics Statistics::operator+(Statistics &a)
{
	Statistics p;
	p.sum = a.sum+sum;
	p.sum2 = a.sum2+sum2;
	p.vmax = (a.vmax > vmax ? a.vmax : vmax);
	p.vmin = (a.vmin < vmin ? a.vmin : vmin);
	p.scount = a.scount+scount;
	return p;
}

inline void Statistics::clk(float f)
{
    scount++;
    sum += f;
    sum2 += f*f;
    if (f < vmin)
        vmin = f;
    if (f > vmax)
        vmax = f;
}

inline float Statistics::ave()
{
    return float(sum/scount);
}

inline float Statistics::stdp()
{
    if (scount < 1)
        return 0;
    float r = (float)(sum2 / scount - (sum / scount) * (sum / scount));
    if (r < 0)
        return 0;
    else
        return sqrt(r);
}

inline float Statistics::std()
{
    if (scount < 2)
        return 0;
    float r = (float)((sum2 - sum * (sum / scount)) / (scount - 1));
    if (r < 0)
        return 0;
    else
        return sqrt(r);
}

inline float Statistics::min()
{
    return vmin;
}

inline float Statistics::max()
{
    return vmax;
}
//----------------------- Statistics  END ----------------
