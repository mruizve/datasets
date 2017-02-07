#include<sys/stat.h>     // struct stat, stat(), S_ISREG()
#include<iostream>
#include "conversions.h"

//#include<mat.h>       // MATLAB
//#include<opencv2/opencv.hpp>

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

	// conversion type identification
	try
	{
		const convert_t input=convert_get_format(argv[1]);
		if( 0>input.delimiter )
		{
			std::cerr << "(EE) " << argv[0] << ": " << "cannot identify the input data format\n";
			return -1;
		}

		const convert_t output=convert_get_format(argv[2]);
		if( 0>output.delimiter )
		{
			std::cerr << "(EE) " << argv[0] << ": cannot identify the output data format\n";
			return -1;
		}

		// text to text conversions (no XML)
		if( input.delimiter && output.delimiter )
		{
			convert_text2text(input,output);
			return 0;
		}

		// we have at least one binary or XML format, need to verify
		// if the user provided the 'varname' command line argument
		if( 4!=argc )
		{
			std::cerr << "(EE) " << argv[0] << ": missing 'varname' argument (required for XML and binary formats)\n";
			return -1;
		}

		// text to XML/binary conversion
		if( input.delimiter )
		{
			switch( output.id )
			{
				case XML: convert_text2xml(input,output,argv[3]); break;
				case DAT: convert_text2dat(input,output,argv[3]); break;
				case MAT: convert_text2mat(input,output,argv[3]); break;
			}

			return 0;
		}

	}
	catch( std::string& error )
	{
		std::cerr << "(EE) " << argv[0] << ": " << error << std::endl;
		return -1;
	}

/*
	// XML/binary to text conversion
	if( dout )
	{
		switch( input )
		{
			case XML: return convert_xml2text(argv[1],din,argv[2],argv[3]);
			case DAT: return convert_dat2text(argv[1],din,argv[2],argv[3]);
			case MAT: return convert_mat2text(argv[1],din,argv[2],argv[3]);
		}
	}

	// XML/binary to XML/binary
	switch( CONVERSION(input,output) )
	{
		case CONVERSION(ASCII,XML):
		case CONVERSION(CSV,XML):
		case CONVERSION(TSV,XML):
			break;

		case CONVERSION(ASCII,DAT):
		case CONVERSION(ASCII,MAT):
		case CONVERSION(CSV,DAT):
		case CONVERSION(CSV,MAT):
			break;
	}

	// prepare for reading data
    cv::FileStorage xml;
    xml.open(argv[1],cv::FileStorage::READ);
    if( !xml.isOpened() )
    {
		std::cerr << "(EE) %s: cannot open the input file '%s'\n",argv[0],argv[1]);
		return -1;
    }

	// read data
	cv::Mat data;
	xml[std::string(argv[2])] >> data;

	// stop reading
    xml.release();

	// prepare for writing data
	std::ofstream txt(argv[3]);
	if( txt.fail() )
	{
		std::cerr << "(EE) %s: cannot create the output file '%s'",argv[0],argv[3]);
		return -1;
	}

	// write data
	for( int i=0; data.rows>i; i++ )
	{
		cv::Mat imrow=data.row(i);
		cv::Mat im=imrow.reshape(1,46);
		cv::imshow("imrow",im);
		cv::waitKey(100);
		// for( int j=0; data.cols>j; j++ )
		// {
			// std::cout << data.at<float>(i,j) << " ";
		// }
		// std::cout << std::endl;
	}
*/
	return 0;
}
