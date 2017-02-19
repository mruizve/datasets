#include<getopt.h>
#include<stdlib.h>
#include<iostream>
#include "dmatrix.h"

static void usage(const char *argv0)
{
	std::cerr <<  "usage: "<< argv0 <<
		" [options] features labels\n\n"
		"   features: path of the features file\n"
		"     labels: path of the labels file\n\n"
		"available options:\n"
		"   -f, --fname name of the features data container (default: 'features')\n"
		"   -l, --lname name of the labels data container (dafault: 'labels')\n"
		"   -b, --bsize CUDA threads block size (default value: 32)\n\n"
		"features and labels data formats are guessed from the corresponding\n"
		"files extensions. when missing or unknown, ASCII format is assumed.\n"
		"--fname or --lname are ignored when the corresponding data is given\n"
		"in text format.\n\n"
		"supported formats are:\n"
		"   .txt, .csv, .tsv, .xml (OpenCV), .dat (tSNE), .mat (MATLAB)\n";
}

int dmOptions(int argc, char *argv[], DMOptions *dm)
{
	// validate input arguments
	struct warning
	{
		bool show;
		std::string message;
	};

	static struct warning warnings[]=
	{
		{1,"name of the features data container was not defined, assuming 'features'"},
		{1,"name of the labels data container was not defined, assuming 'labels'"},
		{1,"CUDA block size set to 32 threads"}
	};

	static struct option options[]=
	{
		{"fname",required_argument,0,'f'},
		{"lname",required_argument,0,'l'},
		{"bsize",required_argument,0,'b'},
		{0,0,0,0}
	};

	// set default values
	dm->bsize=32;
	dm->fname=std::string("features");
	dm->lname=std::string("labels");

	// options loop
	int idx=0;
	int c=getopt_long(argc,argv,"f:l:",options,&idx);
	for( ; -1!=c; c=getopt_long(argc,argv,"f:l:",options,&idx) )
	{
		switch( c )
		{
			case 'f':
				dm->fname=std::string(optarg);
				warnings[idx].show=0;
				break;

			case 'l':
				dm->lname=std::string(optarg);
				warnings[idx].show=0;
				break;

			case 'b':
				dm->bsize=strtol(optarg,NULL,10);
				warnings[idx].show=0;
				break;

			case '?':
			default:
				usage(argv[0]);
				return -1;
		}
	}

	// validate non-optional arguments 
	if( 2!=(argc-optind) )
	{
		usage(argv[0]);
		return -1;
	}

	// show option warnings, if any
	for( size_t i=0; sizeof(warnings)/sizeof(warning)>i; i++ )
	{
		if( warnings[i].show )
		{
			std::cerr << "(WW) " << argv[0] << ": " << warnings[i].message << std::endl;
		}
	}

	// store data paths
	dm->fpath=std::string(argv[optind]);
	dm->lpath=std::string(argv[optind+1]);

	return 0;
}
