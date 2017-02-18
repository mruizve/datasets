#include<cuda.h>
#include<iostream>
#include<iomanip>
#include "dmatrix.h"

__global__ void dmCudaDistancesColMajor
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

__global__ void dmCudaDistancesRowMajor
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
	
	if( indexes->rows<(offsets.size()-1) )
	{
		throw std::string("invalid offsets array");
	}

	try
	{
		// distance matrix initialization
		cv::Mat matrix(offsets.size()-1,offsets.size()-1,CV_32FC4);
		float *raw=(float*)matrix.data;

		// computed distances between all features of the labels pair
		// (remember that count[i]=offsets[i+1]-offsets[i])
		std::cout << "processing distance matrix's rows" << std::endl;

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

				if( IOColMajor==features->ordering )
				{
					dmCudaDistancesColMajor<<<grid,threads>>>(
						d_distances,
						(float*)features->pointer,features->cols,features->rows,
						(int*)indexes->pointer,
						offsets[i],offsets[j],xdim,ydim);
				}
				else
				{
					dmCudaDistancesRowMajor<<<grid,threads>>>(
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

				// compute distances statistics
				float d_mean=0.0f,d_var=0.0f,d_max=0.0f,d_min=1e9;
				for( size_t k=0; h_distances.size()>k; k++ )
				{
					d_mean+=h_distances.at(k);
					d_max=std::max(d_max,h_distances.at(k));
					d_min=std::min(d_min,h_distances.at(k));
				}
				d_mean/=h_distances.size();
				for( size_t k=0; h_distances.size()>k; k++ )
				{
					d_var+=(h_distances.at(k)-d_mean)*(h_distances.at(k)-d_mean);
				}
				d_var/=(h_distances.size()-1);

				// store statistics
				raw[3*(i*matrix.cols+j)+0]=d_mean;
				raw[3*(i*matrix.cols+j)+1]=d_var;
				raw[3*(i*matrix.cols+j)+2]=d_max;
				raw[3*(i*matrix.cols+j)+3]=d_min;

				raw[3*(j*matrix.cols+i)+0]=d_mean;
				raw[3*(j*matrix.cols+i)+1]=d_var;
				raw[3*(j*matrix.cols+i)+2]=d_max;
				raw[3*(j*matrix.cols+i)+3]=d_min;
			}

			std::cout
				<< std::setw(5) << std::setfill('0') << i+1 << "/"
				<< std::setw(5) << std::setfill('0') << offsets.size()-1 <<'\r'
				<< std::flush;
		}
		std::cout << std::endl;

		return matrix;
	}
	catch( const std::string& error )
	{
		throw "cannot generate the distance matrix array ("+error+")";
	}
}
