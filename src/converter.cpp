#include<sys/stat.h>     // struct stat, stat(), S_ISREG()
#include<iostream>
#include "convert/format.h"

int main(int argc, char *argv[])
{
	// validate input arguments
	if( 3!=argc && 4!=argc )
	{
		std::cerr <<  "usage "<< argv[0] <<
			": input output [varname]\n\n"
			"    input: path of the source file\n"
 			"   output: path of the destination file\n"
			"  varname: name of the data container\n\n"
			"the conversion from source to destination is done based on the files\n"
			"extensions. when missing or unknown, ASCII format is assumed. when\n"
			"converting between text formats, the 'varname' argument, if any, is\n"
			"ignored.\n\n"
			"supported formats are:\n"
			"   .txt, .csv, .tsv, .xml (OpenCV), .dat (tSNE), .mat (MATLAB)\n";
		return -1;
	}

	// validate input and output paths
	int err;
    struct stat stats;

    err=stat(argv[1],&stats);
    if( 0>err || !S_ISREG(stats.st_mode) )
    {
		std::cerr << "(EE) " << argv[0] << ": invalid input file '" << argv[1] << "'\n";
		return -1;
	}

    err=stat(argv[2],&stats);
    if( !err )
    {
		// path exists
		// it is a regular file?
		if( S_ISREG(stats.st_mode) )
		{
			// yes, it is. overwrite it?
			std::string line;
			do
			{
				std::cout << "(\?\?) " << argv[0] << ": overwrite the output file '" << argv[2] << "'? ";
			}
			while( std::getline(std::cin,line) && line.compare("y") && line.compare("n") );

			// if not, abort conversion
			if( line.compare("y") )
			{
				std::cerr << "(WW) " << argv[0] << ": conversion aborted as requested\n";
				return -1;
			}
		}
		else
		{
			// no, it is not a regular file
			std::cerr << "(EE) " << argv[0] << ": invalid output file '" << argv[2] << "'\n";
			return -1;
		}
	}

	// open the input file and prepare for read
	Format *input=tryOpen(argv[1],argv[3],Source);
	if( NULL==input )
	{
		std::cerr << "(EE) " << argv[0] << ": " << "cannot identify the input data format\n";
		return -1;
	}
	
	// open the output file and prepare for writing
	Format *output=tryOpen(argv[2],argv[3],Destination);
	if( NULL==output )
	{
		std::cerr << "(EE) " << argv[0] << ": " << "cannot identify the output data format\n";
		return -1;
	}

	try
	{
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

		delete input;
		delete output;
	}
	catch( std::string& error )
	{
		std::cerr << "(EE) " << argv[0] << ": " << error << std::endl;
		return -1;
	}
	catch( ... )
	{
		std::cerr << "(EE) " << argv[0] << ": " << "unexpected exception during data conversion" << std::endl;
		return -1;
	}

	return 0;
}
