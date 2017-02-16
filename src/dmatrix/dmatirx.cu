#include<cuda.h>
#include<iostream>
#include "dmatrix.h"

__global__ void dmCudaDistancesColMajor(float *matrix, size_t mdim, const float *features, size_t fcols, size_t frows, const int *indexes, const int *count)
{
	// compute pixel coordinates
	const int x=blockDim.x*blockIdx.x+threadIdx.x;
	const int y=blockDim.y*blockIdx.y+threadIdx.y;

	#define F(i,j) features[(i)*frows+(j)]
	#define M(i,j,k) matrix[3*((j)*mdim+(i))+k]
	if( mdim>x && mdim>y && x>=y )
	{
		// compute indexes offsets
		int xoffset=0;
		for( int i=0; x>i; i++ )
		{
			xoffset+=count[i];
		}

		int yoffset=0;
		for( int i=0; y>i; i++ )
		{
			yoffset+=count[i];
		}

		// distances initialization
		float d_mean=0.0f,d_max=0.0f,d_min=1e9;

		// compute distances between all images of the labels pair
		for( int i=0; count[x]>i; i++)
		{
			for( int j=0; count[y]>j; j++)
			{
				// compute indexes
				int ii=indexes[xoffset+i];
				int jj=indexes[yoffset+j];

				// compute features distances
				float d=0.0f;
				for( int k=0; fcols>k; k++ )
				{
					d+=(F(k,ii)-F(k,jj))*(F(k,ii)-F(k,jj));
				}
				d=sqrtf(d);

				// accumulate mean
				d_mean+=d;

				// get maximum distance
				d_max=fmaxf(d,d_max);

				// get minimum distance
				d_min=fminf(d,d_min);
			}
		}

		// compute mean
		d_mean=d_mean/(count[x]*count[y]);

		// update matrix values
		M(x,y,0)=d_mean;
		M(x,y,1)=d_max;
		M(x,y,2)=d_min;

		M(y,x,0)=d_mean;
		M(y,x,1)=d_max;
		M(y,x,2)=d_min;
	}
	#undef M
    #undef F
}

__global__ void dmCudaDistancesRowMajor(float *matrix, size_t mdim, const float *features, size_t fcols, size_t frows, const int *indexes, const int *count)
{
	// compute pixel coordinates
	const int x=blockDim.x*blockIdx.x+threadIdx.x;
	const int y=blockDim.y*blockIdx.y+threadIdx.y;

	#define F(i,j) features[(j)*fcols+(i)]
	#define M(i,j,k) matrix[3*((j)*mdim+(i))+k]
	if( mdim>x && mdim>y && x>=y )
	{
		// compute indexes offsets
		int xoffset=0;
		for( int i=0; x>i; i++ )
		{
			xoffset+=count[i];
		}

		int yoffset=0;
		for( int i=0; y>i; i++ )
		{
			yoffset+=count[i];
		}

		// distances initialization
		float d_mean=0.0f,d_max=0.0f,d_min=1e9;

		// compute distances between all images of the labels pair
		for( int i=0; count[x]>i; i++)
		{
			for( int j=0; count[y]>j; j++)
			{
				// compute indexes
				int ii=indexes[xoffset+i];
				int jj=indexes[yoffset+j];

				// compute features distances
				float d=0.0f;
				for( int k=0; fcols>k; k++ )
				{
					d+=(F(k,ii)-F(k,jj))*(F(k,ii)-F(k,jj));
				}
				d=sqrtf(d);

				// accumulate mean
				d_mean+=d;

				// get maximum distance
				d_max=fmaxf(d,d_max);

				// get minimum distance
				d_min=fminf(d,d_min);
			}
		}

		// compute mean
		d_mean=d_mean/(count[x]*count[y]);

		// update matrix values
		M(x,y,0)=d_mean;
		M(x,y,1)=d_max;
		M(x,y,2)=d_min;

		M(y,x,0)=d_mean;
		M(y,x,1)=d_max;
		M(y,x,2)=d_min;
	}
	#undef M
    #undef F
}

DMCudaArray* dmCudaDistanceMatrix(const DMCudaArray *features, const DMCudaArray *indexes, const DMCudaArray *count, size_t bsize)
{
	// validate input arguments
	if( NULL==features )
	{
		throw std::string("invalid features array");
	}

	if( NULL==indexes || 1!=indexes->cols )
	{
		throw std::string("invalid indexes array");
	}

	if( features->rows!=indexes->rows )
	{
		throw std::string("features and indexes arrays should have the same number of elements");
	}
	
	if( NULL==count || 1!=count->cols || indexes->rows<count->rows )
	{
		throw std::string("invalid frequencies array");
	}

	DMCudaArray *matrix=NULL;

	try
	{
		// array initialization
		matrix=new DMCudaArray;
		matrix->cols=count->rows;
		matrix->rows=count->rows;
		matrix->bytes=3*sizeof(float)*matrix->cols*matrix->rows;
		matrix->ordering=IORowMajor;

		cudaASSERT( cudaMalloc(&matrix->pointer,matrix->bytes) );

		dim3 grid(1,1,1);
		dim3 threads(bsize,bsize,1);
		grid.x=(matrix->cols/bsize)+((matrix->cols%bsize)?1:0);
		grid.y=(matrix->rows/bsize)+((matrix->rows%bsize)?1:0);

		if( IOColMajor==features->ordering )
		{
			dmCudaDistancesColMajor<<<grid,threads>>>(
				(float*)matrix->pointer,matrix->cols,
				(float*)features->pointer,features->cols,features->rows,
				(int*)indexes->pointer,(int*)count->pointer);
		}
		else
		{
			dmCudaDistancesRowMajor<<<grid,threads>>>(
				(float*)matrix->pointer,matrix->cols,
				(float*)features->pointer,features->cols,features->rows,
				(int*)indexes->pointer,(int*)count->pointer);
		}

		cudaASSERT( cudaPeekAtLastError() );
		cudaASSERT( cudaDeviceSynchronize() );
	}
	catch( const std::string& error )
	{
		if( NULL!=matrix )
		{
			dmCudaFree(matrix);
		}
		throw "cannot generate the distance matrix array ("+error+")";
	}

	return matrix;
}
