#ifndef __FACEDATA_DMATRIX_H__
#define __FACEDATA_DMATRIX_H__

#include<string>
#include<opencv2/opencv.hpp>
#include "io/files.h"

#define cudaASSERT(ans) \
{ \
	cudaAssert((ans), __FILE__, __FUNCTION__, __LINE__); \
}

typedef struct
{
	size_t bsize;
	std::string fname;
	std::string fpath;
	std::string lname;
	std::string lpath;
} DMOptions;

typedef struct
{
	size_t cols;
	size_t rows;
	size_t bytes;
	void *pointer;
	IOMajor ordering;
} DMArray;

// function for parsing command line arguments
int dmOptions(int argc, char *argv[], DMOptions *dm);

// throw exception when a CUDA error occurs
void cudaAssert(int code, const char *file, const char *fn, int line);

// device arrays initialization
DMArray* dmInitArray(const IOFile *file);

// array sorting (based on Thrust sorting)
DMArray* dmSortArray(const DMArray *keys, size_t bsize);

// unique keys frequency counting
std::vector<int> dmCountUniqueKeys(const DMArray* keys);

// distance matrix computation
cv::Mat dmDistanceMatrix(const DMArray *features, const DMArray *indexes, const std::vector<int> count, size_t bsize);

// free device arrays
void dmFree(DMArray *array);

#endif
