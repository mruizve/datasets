#include<sys/stat.h>     // struct stat, stat(), S_ISREG()
#include<iostream>
#include "conversions.h"

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

		// text to XML/binary conversions
		if( input.delimiter )
		{
			switch( output.id )
			{
				case MY_XML: convert_text2xml(input,output,argv[3]); break;
				case MY_DAT: convert_text2dat(input,output,argv[3]); break;
				case MY_MAT: convert_text2mat(input,output,argv[3]); break;
			}

			return 0;
		}

		// XML/binary to text conversions
		if( output.delimiter )
		{
			switch( input.id )
			{
				case MY_XML: convert_xml2text(input,output,argv[3]); break;
				// case MY_DAT: convert_dat2text(input,output,argv[3]); break;
				case MY_MAT: convert_mat2text(input,output,argv[3]); break;
			}

			return 0;
		}


		// XML/binary to XML/binary conversions
		switch( CONVERSION(input.id,output.id) )
		{
			case CONVERSION(MY_XML,MY_XML): convert_xml2xml(input,output,argv[3]); break;
			case CONVERSION(MY_XML,MY_DAT): convert_xml2dat(input,output,argv[3]); break;
			case CONVERSION(MY_XML,MY_MAT): convert_xml2mat(input,output,argv[3]); break;
			// case CONVERSION(MY_DAT,MY_XML): convert_dat2xml(input,output,argv[3]); break;
			// case CONVERSION(MY_DAT,MY_DAT): convert_dat2dat(input,output,argv[3]); break;
			// case CONVERSION(MY_DAT,MY_MAT): convert_dat2mat(input,output,argv[3]); break;
			case CONVERSION(MY_MAT,MY_XML): convert_mat2xml(input,output,argv[3]); break;
			case CONVERSION(MY_MAT,MY_DAT): convert_mat2dat(input,output,argv[3]); break;
			case CONVERSION(MY_MAT,MY_MAT): convert_mat2mat(input,output,argv[3]); break;
		}

	}
	catch( const std::string& error )
	{
		std::cerr << "(EE) " << argv[0] << ": " << error << std::endl;
		return -1;
	}

/*
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
