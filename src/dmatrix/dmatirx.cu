#include<cuda.h>
#include<iostream>
#include<iomanip>
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

__global__ void dmCudaDMColMajor
(
	float *distances, const float *features, size_t fcols, size_t frows, const int *indexes, size_t xoffset, size_t yoffset, size_t xdim, size_t ydim
)
{
	// compute pixel coordinates
	const int x=blockDim.x*blockIdx.x+threadIdx.x;
	const int y=blockDim.y*blockIdx.y+threadIdx.y;

	#define F(i,j) features[(i)*frows+(j)]
	#define D(i,j) distances[(j)*xdim+(i)]
	if( xdim>x && ydim>y )
	{
		// compute features indexes
		int xx=indexes[xoffset+x];
		int yy=indexes[yoffset+y];
		
		// compute features distances
		float d=0.0f;
		for( int k=0; fcols>k; k++ )
		{
			d+=(F(k,xx)-F(k,yy))*(F(k,xx)-F(k,yy));
		}

		D(x,y)=sqrtf(d);
	}
	#undef D
	#undef F
}

__global__ void dmCudaDMRowMajor
(
	float *distances, const float *features, size_t fcols, size_t frows, const int *indexes, size_t xoffset, size_t yoffset, size_t xdim, size_t ydim
)
{
	// compute pixel coordinates
	const int x=blockDim.x*blockIdx.x+threadIdx.x;
	const int y=blockDim.y*blockIdx.y+threadIdx.y;

	#define F(i,j) features[(j)*fcols+(i)]
	#define D(i,j) distances[(j)*xdim+(i)]
	if( xdim>x && ydim>y )
	{
		// compute features indexes
		int xx=indexes[xoffset+x];
		int yy=indexes[yoffset+y];
		
		// compute features distances
		float d=0.0f;
		for( int k=0; fcols>k; k++ )
		{
			d+=(F(k,xx)-F(k,yy))*(F(k,xx)-F(k,yy));
		}

		D(x,y)=sqrtf(d);
	}
	#undef D
	#undef F
}

//DMCudaArray* dmCudaDistanceMatrix(const DMCudaArray *features, const DMCudaArray *indexes, const DMCudaArray *count, size_t bsize)
cv::Mat dmCudaDistanceMatrix(const DMCudaArray *features, const DMCudaArray *indexes, const std::vector<int> offsets, size_t bsize)
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
	
	// if( NULL==count || 1!=count->cols || indexes->rows<count->rows )
	// {
		// throw std::string("invalid frequencies array");
	// }
	if( indexes->rows<(offsets.size()-1) )
	{
		throw std::string("invalid offsets array");
	}
//	DMCudaArray *matrix=NULL;

	try
	{
/*
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
*/
		cv::Mat matrix(offsets.size()-1,offsets.size()-1,CV_32FC3);

		for( size_t i=0; (offsets.size()-1)>i; i++ )
		{
			size_t xdim=offsets[i+1]-offsets[i];

			for( size_t j=i; (offsets.size()-1)>j; j++ )
			{
				size_t ydim=offsets[j+1]-offsets[j];

				float *d_distances;
				size_t bytes=sizeof(float)*xdim*ydim;
				cudaASSERT( cudaMalloc((void**)&d_distances,bytes) );

				dim3 grid(1,1,1);
				dim3 threads(bsize,bsize,1);
				grid.x=(xdim/bsize)+((xdim%bsize)?1:0);
				grid.y=(ydim/bsize)+((ydim%bsize)?1:0);

				// computed distances between all features of the labels pair
				if( IOColMajor==features->ordering )
				{
					dmCudaDMColMajor<<<grid,threads>>>(
						d_distances,
						(float*)features->pointer,features->cols,features->rows,
						(int*)indexes->pointer,
						offsets[i],offsets[j],xdim,ydim);
				}
				else
				{
					dmCudaDMRowMajor<<<grid,threads>>>(
						d_distances,
						(float*)features->pointer,features->cols,features->rows,
						(int*)indexes->pointer,
						offsets[i],offsets[j],xdim,ydim);
				}

				cudaASSERT( cudaPeekAtLastError() );

				// retrieve distances
				std::vector<float> h_distances(xdim*ydim);
				cudaASSERT( cudaMemcpy(&h_distances[0],d_distances,bytes,cudaMemcpyDeviceToHost) );

				// free memory resources
				cudaASSERT( cudaFree(d_distances) );

				// reduce distances
				float d_mean=0.0f,d_max=0.0f,d_min=1e9;
				for( size_t k=0; h_distances.size()>k; k++ )
				{
					d_mean+=h_distances.at(k);
					d_max=std::max(d_max,h_distances.at(k));
					d_min=std::min(d_min,h_distances.at(k));
				}
				d_mean=d_mean/h_distances.size();

				// store distances
				matrix.at<cv::Vec3f>(i,j)[0]=d_mean;
				matrix.at<cv::Vec3f>(i,j)[1]=d_max;
				matrix.at<cv::Vec3f>(i,j)[2]=d_min;

				matrix.at<cv::Vec3f>(j,i)=matrix.at<cv::Vec3f>(i,j);
			}

			std::cout
				<< std::setw(5) << std::setfill('0') << i << "/"
				<< std::setw(5) << std::setfill('0') << offsets.size()-1 <<'\r'
				<< std::flush;
		}
		std::cout << std::endl;

		return matrix;
/*
		cudaASSERT( cudaPeekAtLastError() );
		cudaASSERT( cudaDeviceSynchronize() );
*/
	}
	catch( const std::string& error )
	{
/*
		if( NULL!=matrix )
		{
			dmCudaFree(matrix);
		}
*/
		throw "cannot generate the distance matrix array ("+error+")";
	}

//	return matrix;
	return cv::Mat();
}
