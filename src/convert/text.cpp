#include<unistd.h>           // execl()
#include<opencv2/opencv.hpp>
#include<algorithm>          // std::replace()
#include<fstream>
#include "conversions.h"

void convert_text2text(const convert_t input, const convert_t output)
{
	// input and output are in the same text format
	if( input.delimiter==output.delimiter )
	{
		execl("/bin/cp","-p",input.path,output.path,NULL);
	}
	else
	{
		// prepare for conversion
		std::ifstream fin(input.path);
		if( fin.fail() )
		{
			throw "cannot open the input file '"+std::string(input.path)+"'";
		}

		std::ofstream fout(output.path);
		if( fout.fail() )
		{
			throw "cannot open the output file '"+std::string(input.path)+"'";
		}

		std::string line;
		while( std::getline(fin,line) )
		{
			// no data but delimiter conversion
			std::replace(line.begin(),line.end(),input.delimiter,output.delimiter);

			// write data
			fout << line << std::endl;
		}
	}
}

void convert_text2xml(const convert_t input, const convert_t output, const char* varname)
{
	// prepare for conversion
	std::ifstream fin(input.path);
	if( fin.fail() )
	{
		throw "cannot open the input file '"+std::string(input.path)+"'";
	}

    cv::FileStorage fout;
    fout.open(output.path,cv::FileStorage::WRITE);
    if( !fout.isOpened() )
    {
		throw "cannot open the output file '"+std::string(output.path)+"'";
    }

	// initialize data structure
	fout << std::string(varname) << "{:";

	// for each input line, do
	std::string line;
	while( std::getline(fin,line) )
	{
		std::string value;
		std::stringstream ss(line);

		// retrieve and write line (or row) values
		fout << "row" << "[:";
		while( std::getline(ss,value,(char)input.delimiter) )
		{
			fout << strtof(value.c_str(),NULL);
		}
		fout << "]";
	}

	// end of data structure
	fout << "}";

	// close the output file
    fout.release();
}

void convert_text2dat(const convert_t input, const convert_t output, const char *varname)
{
}

void convert_text2mat(const convert_t input, const convert_t output, const char *varname)
{
}

