#include<mat.h>              // MATLAB
#include<opencv2/opencv.hpp>
#include<fstream>
#include "conversions.h"

void convert_xml2text(const convert_t input, const convert_t output, const char *varname)
{
	// prepare for conversion
    cv::FileStorage fin;
	try
	{
		fin.open(input.path,cv::FileStorage::READ);
	}
	catch( const cv::Exception& error )
	{
		throw std::string("unexpected exception from cv::FileStorage::open()");
	}
	if( !fin.isOpened() )
	{
		throw "cannot open the input file '"+std::string(input.path)+"'";
	}

	std::ofstream fout(output.path);
	if( fout.fail() )
	{
		throw "cannot open the output file '"+std::string(output.path)+"'";
	}

	// read data
	cv::Mat data;
	fin[std::string(varname)] >> data;
	if( data.empty() || 1!=data.channels() )
	{
		// empty or invalid data (we expect single channel matrices)
		throw "cannot read data from '"+std::string(input.path)+"'";
	}

	// for each data element,
	for( size_t row=0; data.rows>row; row++ )
	{
		for( size_t col=0; data.cols-1>col; col++ )
		{
			// write values to the output file
			fout << data.at<float>(row,col) << (char)output.delimiter;
		}
		fout << data.at<float>(row,data.cols-1) << '\n';
	}

	// close the file streams
    fin.release();
    fout.close();
}

void convert_xml2xml(const convert_t input, const convert_t output, const char *varname)
{
	// prepare for conversion
    cv::FileStorage fin,fout;
	try
	{
		fin.open(input.path,cv::FileStorage::READ);
		fout.open(output.path,cv::FileStorage::WRITE);
	}
	catch( const cv::Exception& error )
	{
		throw std::string("unexpected exception from cv::FileStorage::open()");
	}
	if( !fin.isOpened() )
	{
		throw "cannot open the input file '"+std::string(input.path)+"'";
	}
	if( !fout.isOpened() )
	{
		throw "cannot open the output file '"+std::string(output.path)+"'";
	}

	// load data
	cv::Mat data;
	fin[std::string(varname)] >> data;
	if( data.empty() || 1!=data.channels() )
	{
		// empty or invalid data (we expect single channel matrices)
		throw "cannot read data from '"+std::string(input.path)+"'";
	}

	// write data
	fout << std::string(varname) << data;

	// close the file streams
    fin.release();
    fout.release();
}

void convert_xml2dat(const convert_t input, const convert_t output, const char *varname)
{
	throw std::string("XML to .dat conversion still under development");
}

void convert_xml2mat(const convert_t input, const convert_t output, const char *varname)
{
	throw std::string("XML to .mat conversion still under development");
}
