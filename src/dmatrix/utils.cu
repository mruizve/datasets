#include<sstream>
#include<cuda.h>

// throw exception when a CUDA error occurs
void cudaAssert(int code, const char *file, const char *fn, int line)
{
	if( cudaSuccess!=code ) 
	{
		std::stringstream ss;
		ss << cudaGetErrorString((cudaError_t)code) << " at " << file << ":" << line;
		throw ss.str();
	}
}
