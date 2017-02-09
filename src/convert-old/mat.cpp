#include<mat.h>              // MATLAB
#include<opencv2/opencv.hpp>
#include<fstream>
#include "conversions.h"

void convert_mat2text(const convert_t input, const convert_t output, const char *varname)
{
	// prepare for conversion
	MATFile *fin=matOpen(input.path,"r");
	if( NULL==fin )
	{
		throw "cannot open the input file '"+std::string(input.path)+"'";
	}

	std::ofstream fout(output.path);
	if( fout.fail() )
	{
		throw "cannot open the output file '"+std::string(output.path)+"'";
	}

	// read data format
	mxArray *data=matGetVariable(fin,varname);
	if( NULL==data || 2!=mxGetNumberOfDimensions(data) || mxSINGLE_CLASS!=mxGetClassID(data) || mxIsComplex(data) )
	{
		// empty or invalid data (we expect a two dimensional real single precision -float- matrix)
		throw "cannot read data from '"+std::string(input.path)+"'";
	}

	// get a pointer to the data buffer
	float *raw=(float*)mxGetData(data);
	if( NULL==raw )
	{
		throw std::string("unexpected error from mxGetData()'");
	}

	// count number of data rows and columns
	size_t rows=mxGetM(data);
	size_t cols=mxGetN(data);

	// for each data element,
	for( size_t row=0; rows>row; row++ )
	{
		for( size_t col=0; cols-1>col; col++ )
		{
			// write values to the output file
			fout << *(raw+col*rows+row) << (char)output.delimiter;
		}
		fout << *(raw+(cols-1)*rows+row) << '\n';
	}

	// release data and close file streams
	fout.close();

	mxDestroyArray(data);

	if( matClose(fin) )
	{
		throw std::string("unexpected error from matClose()");
		return;
	}
}

void convert_mat2xml(const convert_t input, const convert_t output, const char *varname)
{
	throw std::string(".mat to XML conversion still under development");
/*
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

	// write data
	fout << std::string(varname) << data;

	// close the file streams
    fin.release();
    fout.release();
*/
}

void convert_mat2dat(const convert_t input, const convert_t output, const char *varname)
{
	throw std::string(".mat to .dat conversion still under development");
}

void convert_mat2mat(const convert_t input, const convert_t output, const char *varname)
{
	throw std::string(".mat to .mat conversion still under development");
}
