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
} DMCudaArray;

// function for parsing command line arguments
int dmOptions(int argc, char *argv[], DMOptions *dm);

// throw exception when a CUDA error occurs
void cudaAssert(int code, const char *file, const char *fn, int line);

// device arrays initialization
DMCudaArray* dmCudaInitArray(const IOFile *file);

// array sorting (based on Thrust sorting)
DMCudaArray* dmCudaSortArray(const DMCudaArray *keys, size_t bsize);

// unique keys frequency counting
std::vector<int> dmCudaCountKeys(const DMCudaArray* keys);

// distance matrix computation
cv::Mat dmCudaDistanceMatrix(const DMCudaArray *features, const DMCudaArray *indexes, const std::vector<int> count, size_t bsize);

// free device arrays
void dmCudaFree(DMCudaArray *array);

#endif
