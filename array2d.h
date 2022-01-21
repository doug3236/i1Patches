#pragma once

#include <vector>
#include <string>
#include <array>
#include <cmath>
#include <numeric>
#include <algorithm>


extern "C"
{
    // Use littleCMS to process LAB array to RGB device values, returns false if no profile
    int convert_device_rgb_to_lab_mem(const void* profilemem, int prof_len, const float rgb[][3], float lab[][3], int len, int colorimetric_mode);
    int convert_lab_to_device_rgb_mem(const void* profilemem, int prof_len, const float lab[][3], float rgb[][3], int len, int colorimetric_mode);
    int is_profile_valid(const char* profile);
}


// General 2D array suitable for working with single color or B&W images
template <class T>
class Array2D {
public:
	// used for sub-image boundaries
	struct Extants {
		int top;
		int bottom;
		int left;
		int right;
	};
	std::vector<T> v;
	int nr;
    int nc;
    Array2D(int NR = 0, int NC = 0) : v(NR* NC), nc(NC), nr(NR) {}
	Array2D(int NR, int NC, T val) : v(NR* NC, val), nc(NC), nr(NR) {}
	Array2D(int NR, int NC, T* val, bool transpose);	// transposed=true to switch from col major to row major

	// row and col access via [row][col] or (row,col)
	T* operator[](int r) { return &v[r * nc]; }
	const T* operator[](int r) const { return &v[r * nc]; }
	T& operator()(int r, int c) { return v[r * nc + c]; }
	const T& operator()(int r, int c) const { return v[r * nc + c]; }

	// used to apply/remove gamma
	void pow(float power) { std::transform(v.begin(), v.end(), v.begin(), [power](T x) {return std::pow(x, power); }); }
	T ave() { return std::accumulate(v.begin(), v.end(), T{ 0 }) / v.size(); };
	void fill(T val) { std::generate(v.begin(), v.end(), [val]() { return val; }); }
    void fill(T val, int r, int rlen, int c, int clen);
	
	Array2D<T> clip(Extants bounds);	// clip array to new dimensions, ends of Extants are included
	void scale(T factor) { std::transform(v.begin(), v.end(), v.begin(), [factor](T x) {return x * factor; }); }
	Array2D<T> extract(int rowstart, int rlen, int colstart, int clen);	// extract subarray
	void insert(const Array2D<T>& from, int start_row, int start_col);	// insert subarray
    void insert(T val, Extants box);	                                // value into insert subarray
    void print(std::string filename, bool normalize = true) const;		// for debugging, print array as fixed text

	// copy Array2D<float> to a fixed size 2D array<array>>
	template<int N, int M>
	std::array<std::array<float, N>, M> copy_to_array();
};

template<class T>
Array2D<T> transpose(const Array2D<T>& x)
{
	Array2D<T> ret(x.nc, x.nr);
	for (int row = 0; row < ret.nr; row++)
		for (int col = 0; col < ret.nc; col++)
			ret[row][col] = x[col][row];
	return ret;
}


template<class T>
void Array2D<T>::fill(T val, int r, int rlen, int c, int clen)
{
    for (int row = r; row < r+rlen; row++)
        for (int col = c; col < c + clen; col++)
            (*this)[row][col] = val;
}


// create from ptr to elements, with row/col major selection
template<class T>
Array2D<T>::Array2D(int NR, int NC, T* val, bool transpose) :v(NR* NC), nr(NR), nc(NC)
{
	if (transpose)
		for (int i = 0; i < NR; i++)
			for (int ii = 0; ii < NC; ii++)
				(*this)(i, ii) = val[i * NC + ii];
	else
		for (int i = 0; i < NC; i++)
			for (int ii = 0; ii < NR; ii++)
				(*this)(ii, i) = val[i * NR + ii];
}

// print 2D array for further analysis
template<class T>
void Array2D<T>::print(std::string filename, bool normalize) const {
	FILE* fp = fopen(filename.c_str(), "wt");
	T maxv = normalize ? *std::max_element(v.begin(), v.end()) : 1.0f;
	for (int i = 0; i < nr; i++)
	{
		for (int ii = 0; ii < nc; ii++)
			fprintf(fp, "%8.5f ", this->operator()(i, ii) / maxv);
		fprintf(fp, "\n");
	}
	fclose(fp);
}


// Extract a 2D array subset
template<class T>
Array2D<T> Array2D<T>::extract(int rowstart, int rlen, int colstart, int clen)
{
	Array2D ret(rlen, clen);
	for (int r = 0; r < rlen; r++)
		for (int c = 0; c < clen; c++)
			ret(r, c) = (*this)(rowstart + r, colstart + c);
	return ret;
}

// Insert a 2D array into another 2D array
template <class T>
void Array2D<T>::insert(const Array2D<T>& from, int start_row, int start_col)
{
	if (nr < from.nr + start_row || nc < from.nc + start_col)
		throw "Array2D.insert, Out of bounds";
	for (int r = 0; r < from.nr; r++)
		for (int c = 0; c < from.nc; c++)
			(*this)(r + start_row, c + start_col) = from(r, c);
}

// value into insert subarray
template <class T>
void Array2D<T>::insert(T val, Extants box)
{
    for (int r = box.top; r < box.bottom; r++)
        for (int c = box.left; c < box.right; c++)
            (*this)(r, c) = val;
}


// Copy from Array2D on heap to fixed dim array<array<>>, dims must match
template <class T>
template<int N, int M>
std::array<std::array<float, N>, M> Array2D<T>::copy_to_array()
{
	if (nr != N || nc != M)
		throw "array dimensions must match";
	std::array<std::array<float, N>, M> ret;
	for (int r = 0; r < nr; r++)
		for (int c = 0; c < nc; c++)
			ret[r][c] = (*this)(r, c);
	return ret;
}

// Clip to subset as set in Extants
template <typename T>
Array2D<T> Array2D<T>::clip(Extants bounds) {
	Array2D<T> ret(bounds.bottom - bounds.top + 1, bounds.right - bounds.left + 1);
	for (int r = 0; r < ret.nr; r++)
		for (int c = 0; c < ret.nc; c++)
			ret(r, c) = (*this)(r + bounds.top, c + bounds.left);
	return ret;
}
