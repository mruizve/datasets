#include<getopt.h>
#include<iostream>
#include "io/files.h"

void usage(const char *argv0)
{
	std::cerr <<  "usage "<< argv0 <<
		": [options] input output\n\n"
		"    input: path of the source file\n"
		"   output: path of the destination file\n\n"
		"available options:\n"
		"   -i, --iname name of the input data container\n"
		"   -o, --oname name of the output data container\n"
		"   -v, --vname same name for the input and output data containers\n\n"
		"the conversion from source to destination is done based on the files\n"
		"extensions. when missing or unknown, ASCII format is assumed. when\n"
		"converting between text formats, the --iname and --oname options are\n"
		"ignored.\n\n"
		"supported formats are:\n"
		"   .txt, .csv, .tsv, .xml (OpenCV), .dat (tSNE), .mat (MATLAB)\n";
}

int main(int argc, char *argv[])
{
	std::string iname;
	std::string oname;

	// validate input arguments
	static struct option options[] =
	{
		{"iname",required_argument,0,'i'},
		{"oname",required_argument,0,'o'},
		{"vname",required_argument,0,'v'},
		{0,0,0,0}
	};

	int idx=0;
	int c=getopt_long(argc,argv,"i:o:",options,&idx);
	for( ; -1!=c; c=getopt_long(argc,argv,"i:o:",options,&idx) )
	{
		switch( c )
		{
			case 'i': iname=std::string(optarg); break;
			case 'o': oname=std::string(optarg); break;
			case 'v':
				iname=std::string(optarg);
				oname=std::string(optarg);
				break;
			case '?':
			default:
				usage(argv[0]);
				return -1;
		}
	}

	if( 2!=(argc-optind) )
	{
		usage(argv[0]);
		return -1;
	}

	// show warning when variable names are not defined
	if( iname.empty() )
	{
		std::cerr << "(WW) " << argv[0] << ": " << "name of the input data container was not defined\n";
	}
	if( oname.empty() )
	{
		std::cerr << "(WW) " << argv[0] << ": " << "name of the output data container was not defined\n";
	}

	int err=0;
	IOFile *input=NULL;
	IOFile *output=NULL;

	try
	{
		// open the input file and prepare for reading
		IOFile *input=FileOpen(argv[optind],iname.c_str(),IOSource);
		if( NULL==input )
		{
			throw std::string("cannot identify the input file format");
		}
		
		// open the output file and prepare for writing
		IOFile *output=FileOpen(argv[optind+1],oname.c_str(),IODestination);
		if( NULL==output )
		{
			throw std::string("cannot identify the output file format");
		}

		// initialize output data container
		output->initialize(input);

		// convert data values
		for( int row=0; input->getRows()>row; row++ )
		{
			for( int col=0; input->getCols()>col; col++ )
			{
				float value=input->getValue(row,col);
				output->setValue(row,col,value);
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
		std::cerr << "(EE) " << argv[0] << ": " << "unexpected exception during data conversion" << std::endl;
		err=-1;
	}

	if( NULL!=input )
	{
		delete input;
	}

	if( NULL!=output )
	{
		delete output;
	}

	return err;
}
