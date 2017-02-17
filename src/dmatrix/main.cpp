#include<iostream>
#include "dmatrix.h"
#include "io/files.h"

int main(int argc, char *argv[])
{
	// validate command line arguments
	DMOptions dm;
	if( 0>dmOptions(argc,argv,&dm) )
	{
		return -1;
	}

	int err=0;
	IOFile *features=NULL;
	IOFile *labels=NULL;

	try
	{
		// open the features file and prepare for reading
		features=FileOpen(dm.fpath.c_str(),dm.fname.c_str(),IOSource);
		if( NULL==features )
		{
			throw std::string("cannot identify the features file format");
		}
		
		// open the labels file and prepare for reading
		labels=FileOpen(dm.lpath.c_str(),dm.lname.c_str(),IOSource);
		if( NULL==labels )
		{
			throw std::string("cannot identify the labels file format");
		}

		// validate data records
		if( features->getRows()!=labels->getRows() )
		{
			throw std::string("number of features and labels elements mismatch");
		}

		if( 1!=labels->getCols() )
		{
			throw std::string("multi-dimensional labels are not supported");
		}

		// create and initialize the features cuda array and free host resources
		DMCudaArray *dm_features=dmCudaInitArray(features);

		delete features;
		features=NULL;

		// create and initialize the labels cuda array and free host resources
		DMCudaArray *dm_labels=dmCudaInitArray(labels);

		delete labels;
		labels=NULL;

		// create and initialize the indexes cuda array
		DMCudaArray *dm_indexes=dmCudaSortArray(dm_labels,dm.bsize);

		// count unique labels (identities)
		std::vector<int> count=dmCudaCountKeys(dm_labels);

		#ifdef DEBUGGING
			std::cout << "count:" << std::endl;
			for( size_t i=0; count.size()>i; i++ )
			{
				std::cout << count.at(i) << ", ";
			}
			std::cout << "size=" << count.size() << std::endl << std::endl;
		#endif

		// release labels array
		dmCudaFree(dm_labels);

		// compute indexes offsets
		std::vector<int> offsets;
		offsets.push_back(0);
		for( size_t i=0; count.size()>i; i++ )
		{
			offsets.push_back(offsets.back()+count.at(i));
		}

		#ifdef DEBUGGING
			std::cout << "offsets:" << std::endl;
			for( size_t i=0; offsets.size()>i; i++ )
			{
				std::cout << offsets[i] << ", ";
			}
			std::cout << "size=" << offsets.size() << std::endl << std::endl;
		#endif

		// compute distance matrix
		cv::Mat matrix=dmCudaDistanceMatrix(dm_features,dm_indexes,offsets,dm.bsize);

		// release cuda arrays
		dmCudaFree(dm_features);
		dmCudaFree(dm_indexes);

		// export the distance matrix
		std::vector<cv::Mat> channels;
		cv::split(matrix,channels);
		cv::FileStorage file;
		file.open("dmatrix.xml",cv::FileStorage::WRITE);
		file << "mean" << channels[0];
		file << "max" << channels[1];
		file << "min" << channels[2];
		file.release();

		#ifdef DEBUGGING
			std::cout << std::endl << "distance matrix:" << std::endl;
			std::cout << channels[0] << std::endl << std::endl;
			std::cout << channels[1] << std::endl << std::endl;
			std::cout << channels[2] << std::endl << std::endl;
		#endif

		// convert to image
		double min,max;
		cv::minMaxLoc(channels[0],&min,&max);
		cv::convertScaleAbs(channels[0],channels[0],255.0/max,0);
		cv::imwrite("dmatrix-mean.png",channels[0]);

		cv::minMaxLoc(channels[1],&min,&max);
		cv::convertScaleAbs(channels[1],channels[1],255.0/max,0);
		cv::imwrite("dmatrix-max.png",channels[1]);

		cv::minMaxLoc(channels[2],&min,&max);
		cv::convertScaleAbs(channels[2],channels[2],255.0/max,0);
		cv::imwrite("dmatrix-min.png",channels[2]);

		cv::minMaxLoc(matrix,&min,&max);
		cv::convertScaleAbs(matrix,matrix,255.0/max,0);
		cv::imwrite("dmatrix.png",matrix);

		#ifdef DEBUGGING
			std::cout << "before normalization: min=" << min << ", max=" << max << std::endl;
			cv::minMaxLoc(matrix,&min,&max);
			std::cout << "after normalization: min=" << min << ", max=" << max << std::endl;
			cv::imshow("boh",matrix);
			cv::waitKey(0);
		#endif


	}
	catch( std::string& error )
	{
		std::cerr << "(EE) " << argv[0] << ": " << error << std::endl;
		err=-1;
	}
	catch( ... )
	{
		std::cerr << "(EE) " << argv[0] << ": " << "unexpected exception during data conversion" << std::endl;
		err=-1;
	}

	if( NULL!=features )
	{
		delete features;
	}

	if( NULL!=labels )
	{
		delete labels;
	}

	return err;
}
