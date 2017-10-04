#include<getopt.h>
#include<opencv2/opencv.hpp>
#include<sstream>
#include<iomanip>
#include<iostream>
#include "io/files.h"

void usage(const char *argv0)
{
	std::cerr <<  "usage: "<< argv0 <<
		" [options] source1 varname1 ... sourceN varnameN\n\n"
		"     sourceX: path of a source channel\n"
		"    varnameX: name of the data container\n\n"
		"available options:\n"
		"   -1, --one show each channel independently (default)\n"
		"   -3, --three combine triplets of images into a single BGR one\n"
		"   -s, --single-norm normalize each channel independently (default)\n"
		"   -m, --multi-norm normalize all channels all together\n"
		"   -e, --export do not show and save as PNG image (--resize will be ignored)\n"
		"   -r, --resize=N resize when rows>N or cols>N (default no resize)\n\n"
		"data formats are guessed from the sources files extensions. when\n"
		"missing or unknown, ASCII format is assumed. varnameN is ignored\n"
		"when the corresponding sourceN is given in text format.\n\n"
		"supported formats are:\n"
		"   .txt, .csv, .tsv, .xml (OpenCV), .dat (tSNE), .mat (MATLAB)\n";
}

int main(int argc, char *argv[])
{
	bool save=0;
	bool combine=0;
	bool normalize=0;
	double limit=0.0;
	
	// validate input arguments
	static struct option options[] =
	{
		{"one",no_argument,0,'1'},
		{"three",no_argument,0,'3'},
		{"single-norm",no_argument,0,'s'},
		{"multi-norm",no_argument,0,'m'},
		{"export",no_argument,0,'e'},
		{"resize",required_argument,0,'r'},
		{0,0,0,0}
	};

	int idx=0;
	int c=getopt_long(argc,argv,"13smer:",options,&idx);
	for( ; -1!=c; c=getopt_long(argc,argv,"13smer:",options,&idx) )
	{
		switch( c )
		{
			case '1': combine=0; break;
			case '3': combine=1; break;
			case 's': normalize=0; break;
			case 'm': normalize=1; break;
			case 'e': save=1; break;
			case 'r': limit=strtod(optarg,NULL); break;
			case '?':
			default:
				usage(argv[0]);
				return -1;
		}
	}

	if( 2>(argc-optind) || (argc-optind)%2 )
	{
		usage(argv[0]);
		return -1;
	}

	int err=0;

	try
	{
		std::vector<cv::Mat> channels;
		for( int i=optind; i<argc; i+=2 )
		{
			// open the source file and prepare for reading
			IOFile *source=FileOpen(argv[i],argv[i+1],IOSource);
			if( NULL==source )
			{
				throw "unknown data format of '"+std::string(argv[i])+"'";
			}

			// create data container and transfer data to it
			cv::Mat data;
			if( IORowMajor==source->getMajorOrdering() )
			{
				data=cv::Mat(source->getRows(),source->getCols(),CV_32FC1);
				std::memcpy(data.data,source->getDataPtr(),source->getBytes());
			}
			else
			{
				data=cv::Mat(source->getCols(),source->getRows(),CV_32FC1);
				std::memcpy(data.data,source->getDataPtr(),source->getBytes());
				cv::transpose(data,data);
			}

			// normalize each channel independently?
			if( !normalize )
			{
				double min,max,scale;
				cv::minMaxLoc(data,&min,&max); scale=255.0/(max-min);
				cv::convertScaleAbs(data,data,scale,-scale*min);
			}

			// add channel to the list
			channels.push_back(data);

			// delete source
			delete source;
		}

		// create visualization windows
		std::string winname("visualizer -- press ESC to close");
		cv::namedWindow(winname,cv::WINDOW_NORMAL);

		for( size_t i=0,j=0; channels.size()>i; )
		{
			cv::Mat data;

			// combine?
			if( combine && 3<=(channels.size()-i) )
			{
				// merge channels
				std::vector<cv::Mat> aux(channels.begin()+i,channels.begin()+i+3);
				cv::merge(aux,data);

				if( normalize )
				{
					double min,max,scale;
					cv::minMaxLoc(data,&min,&max); scale=255.0/(max-min);
					cv::convertScaleAbs(data,data,scale,-scale*min);
				}
				
				// increment channels index
				i+=3;
			}
			else
			{
				// get channel
				data=cv::Mat(channels[i]);

				// increment channels index
				i++;
			}

			// show or save as PNG?
			if( save )
			{
				// save as PNG image
				std::stringstream ss;
				ss << "image-" << std::setw(5) << std::setfill('0') << j << ".png";

				cv::imwrite(ss.str(),data);

				// increment image index
				j++;
			}
			else
			{
				// resize if required
				if( 0.0!=limit && (limit<data.cols || limit<data.rows) )
				{
					double rr=limit/(double)data.rows;
					double cr=limit/(double)data.cols;
					double r=std::min(rr,cr);

					cv::resize(data,data,cv::Size(r*data.rows,r*data.cols));
				}

				// show image
				char key=0;
				for( char key=0; 0x1b!=key; )
				{
					cv::imshow(winname,data);
					key=cv::waitKey(0);
				}
			}
		}
	}
	catch( std::string& error )
	{
		std::cerr << "(EE) " << argv[0] << ": " << error << std::endl;
		err=-1;
	}
	catch( ... )
	{
		std::cerr << "(EE) " << argv[0] << ": " << "unexpected exception during data visualization" << std::endl;
		err=-1;
	}

	return err;
}
