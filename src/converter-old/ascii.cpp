#include<mat.h>              // MATLAB
#include<unistd.h>           // execl()
#include<opencv2/opencv.hpp>
#include<algorithm>          // std::replace(), std::count()
#include<fstream>
#include "conversions.h"

// taken from
//   http://stackoverflow.com/questions/3072795/how-to-count-lines-of-a-file-in-c
size_t convert_count_rows(std::istream& fs)
{
	// get current state of the stream
    std::istream::iostate state=fs.rdstate();
    std::istream::streampos position=fs.tellg();

    // clear state
    fs.clear();
    fs.seekg(0);

	// count the occurrences of the '\n' character
    size_t rows=std::count(std::istreambuf_iterator<char>(fs),std::istreambuf_iterator<char>(),'\n');

	// we need to add one line to the count if the last character is not '\n'
    fs.unget();
    if( '\n'!=fs.get() )
    {
		++rows;
	}

    // restore the stream state
	fs.clear();
	fs.seekg(position);
	fs.setstate(state);

	return rows;
}

size_t convert_count_cols(std::istream& fs, const int delimiter)
{
	// get current state of the stream
    std::istream::iostate state=fs.rdstate();
    std::istream::streampos position=fs.tellg();

    // clear state
    fs.clear();
    fs.seekg(0);

	// count number of data columns (on the first line)
	size_t cols;
	std::string line;
    std::getline(fs,line);
	std::stringstream ss(line);
	for(cols=0; std::getline(ss,line,(char)delimiter); ++cols);

    // restore the stream state
	fs.clear();
	fs.seekg(position);
	fs.setstate(state);

	return cols;
}

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

		// for each input line, do
		std::string line;
		while( std::getline(fin,line) )
		{
			// no data but delimiter conversion
			std::replace(line.begin(),line.end(),input.delimiter,output.delimiter);

			// write data
			fout << line << std::endl;
		}

		// close the file streams
		fin.close();
		fout.close();
	}
}

void convert_text2xml(const convert_t input, const convert_t output, const char *varname)
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

	// count number of data rows and columns
	size_t rows=convert_count_rows(fin);
	size_t cols=convert_count_cols(fin,input.delimiter);

	// initialize data structure
	cv::Mat data(rows,cols,CV_32FC1);

	// for each input line, do
	std::string line;
	for( size_t row=0; std::getline(fin,line); row++ )
	{
		// get data row
		std::string value;
		std::stringstream ss(line);

		// extract values
		size_t col=0;
		while( std::getline(ss,value,(char)input.delimiter) )
		{
			data.at<float>(row,col)=strtof(value.c_str(),NULL);
			col++;
		}
	}

	// write data
	fout << std::string(varname) << data;

	// close the file streams
	fin.close();
    fout.release();
}

void convert_text2dat(const convert_t input, const convert_t output, const char *varname)
{
	throw std::string("text to .dat conversion still under development");
}

void convert_text2mat(const convert_t input, const convert_t output, const char *varname)
{
	// prepare for conversion
	std::ifstream fin(input.path);
	if( fin.fail() )
	{
		throw "cannot open the input file '"+std::string(input.path)+"'";
	}

	MATFile *fout=matOpen(output.path,"w");
	if( NULL==fout )
	{
		throw "cannot open the output file '"+std::string(output.path)+"'";
	}

	// count number of data rows and columns
	size_t rows=convert_count_rows(fin);
	size_t cols=convert_count_cols(fin,input.delimiter);

	// initialize data structure
	mxArray *data=mxCreateNumericMatrix(rows,cols,mxSINGLE_CLASS,mxREAL);
	if( NULL==data )
	{
		throw std::string("unexpected error from mxCreateNumericMatrix()'");
	}

	// get a pointer to the data buffer
	float *raw=(float*)mxGetData(data);
	if( NULL==raw )
	{
		throw std::string("unexpected error from mxGetData()'");
	}

	// for each input line, do
	std::string line;
	for( size_t row=0; std::getline(fin,line); row++ )
	{
		// get data row
		std::string value;
		std::stringstream ss(line);

		// extract values
		size_t col=0;
		while( std::getline(ss,value,(char)input.delimiter) )
		{
			*(raw+col*rows+row)=strtof(value.c_str(),NULL);
			col++;
		}
	}

	// write data
	if( matPutVariable(fout,varname,data) )
	{
		throw std::string("unexpected error from matPutVariable()'");
	}  
  
	// release data and close the file streams
	fin.close();

	mxDestroyArray(data);

	if( matClose(fout) )
	{
		throw std::string("unexpected error from matClose()");
	}
}
