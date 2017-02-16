#include<cuda.h>
#include<thrust/device_ptr.h>
#include<thrust/device_vector.h>
#include<thrust/sort.h>
#include "dmatrix.h"

DMCudaArray* dmCudaInitArray(const IOFile *file)
{
	// validate input arguments
	if( NULL==file )
	{
		throw std::string("invalid IOFile object");
	}

	DMCudaArray *array=NULL;

	try
	{
		array=new DMCudaArray;
		array->cols=file->getCols();
		array->rows=file->getRows();
		array->bytes=sizeof(float)*array->cols*array->rows;
		array->ordering=file->getMajorOrdering();

		cudaASSERT( cudaMalloc(&array->pointer,array->bytes) );
		cudaASSERT( cudaMemcpy(array->pointer,file->getDataPtr(),array->bytes,cudaMemcpyHostToDevice) );
	}
	catch( const std::string& error )
	{
		if( NULL!=array )
		{
			dmCudaFree(array);
		}
		throw "cannot initialize the CUDA array ("+error+")";
	}

	return array;
}

__global__ void dmCudaInitIndexes(int *indexes, size_t numel)
{
	// compute index
	const int x=blockDim.x*blockIdx.x+threadIdx.x;

    if( numel>x )
    {
		// assign index value
		indexes[x]=x;
    }
}

DMCudaArray* dmCudaSortArray(const DMCudaArray *keys, size_t bsize)
{
	// validate input arguments
	if( NULL==keys || 1!=keys->cols )
	{
		throw std::string("invalid keys array");
	}

	DMCudaArray *indexes=NULL;

	try
	{
		indexes=new DMCudaArray;
		indexes->cols=1;
		indexes->rows=keys->rows;
		indexes->bytes=sizeof(int)*indexes->cols*indexes->rows;
		indexes->ordering=IOColMajor;

		cudaASSERT( cudaMalloc(&indexes->pointer,indexes->bytes) );

		dim3 grid(1,1,1);
		dim3 threads(bsize,1,1);
		grid.x=(indexes->rows/bsize)+((indexes->rows%bsize)?1:0);

		dmCudaInitIndexes<<<grid,threads>>>((int*)indexes->pointer,indexes->rows);
		cudaASSERT( cudaPeekAtLastError() );
		cudaASSERT( cudaDeviceSynchronize() );

		thrust::device_ptr<float> t_keys((float*)keys->pointer);
		thrust::device_ptr<int> t_values((int*)indexes->pointer);
		thrust::sort_by_key(t_keys,t_keys+keys->rows,t_values);
	}
	catch( const std::string& error )
	{
		if( NULL!=indexes )
		{
			dmCudaFree(indexes);
		}
		throw "cannot initialize the indexes array ("+error+")";
	}

	return indexes;
}

DMCudaArray* dmCudaCountKeys(const DMCudaArray *keys)
{
	// validate input arguments
	if( NULL==keys || 1!=keys->cols )
	{
		throw std::string("invalid keys array");
	}

	// temporary copy keys to the host
	float *aux=new float[keys->rows];
	cudaASSERT( cudaMemcpy(aux,keys->pointer,keys->bytes,cudaMemcpyDeviceToHost) );

	// count unique keys and compute frequencies
	int i=0,j=1;
	std::vector<int> frequencies;
	for( ; keys->rows-1>i; i++,j++ )
	{
		if( (aux[i+1]-aux[i]) )
		{
			frequencies.push_back(j);
			j=0;
		}
	}
	frequencies.push_back(j);
/*
for( int i=0; frequencies.size()>i; i++ )
{
	std::cout << frequencies[i] << ", ";
}
std::cout << frequencies.size() << std::endl;
*/
	DMCudaArray *count=NULL;

	try
	{
		count=new DMCudaArray;
		count->cols=1;
		count->rows=frequencies.size();
		count->bytes=sizeof(int)*count->cols*count->rows;
		count->ordering=IOColMajor;

		cudaASSERT( cudaMalloc(&count->pointer,count->bytes) );
		cudaASSERT( cudaMemcpy(count->pointer,&frequencies[0],count->bytes,cudaMemcpyHostToDevice) );
	}
	catch( const std::string& error )
	{
		if( NULL!=count )
		{
			dmCudaFree(count);
		}
		throw "cannot initialize keys frequencies array ("+error+")";
	}

	return count;
}

void dmCudaFree(DMCudaArray *array)
{
	if( NULL!=array )
	{
		if( NULL!=array->pointer )
		{
			// release the cuda array
			cudaASSERT( cudaFree(array->pointer) );
		}

		// clear memory resources
		std::memset(array,sizeof(array),0);

		// release memory resources
		delete array;
	}
}
