#include<algorithm>        // std::count()
#include<sstream>          // string stream
#include "convert/ascii.h"

// taken from
//   http://stackoverflow.com/questions/3072795/how-to-count-lines-of-a-file-in-c
static size_t convert_count_rows(std::fstream& fs)
{
	// get current state of the stream
    std::fstream::iostate state=fs.rdstate();
    std::fstream::streampos position=fs.tellg();

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

static size_t convert_count_cols(std::fstream& fs, int delimiter)
{
	// get current state of the stream
    std::fstream::iostate state=fs.rdstate();
    std::fstream::streampos position=fs.tellg();

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

FileASCII::FileASCII(const char *filename, const char *varname, IOType type)
{
	// load supported formats
	this->loadFormats();

	// prepare for data reading or writing
	this->open(filename,varname,type);
}

FileASCII::~FileASCII(void)
{
	// close the file stream
	this->file.close();
}

void FileASCII::open(const char *filename, const char *varname, IOType _type)
{
	if( !this->tryOpen(filename) )
	{
		throw "'"+std::string(filename)+"' is not a valid text file";
	}

	// open the file stream
	this->file.open(filename,(Source==_type)?(std::ios::in):(std::ios::out|std::ios::trunc));
	if( this->file.fail() )
	{
		throw "cannot open the file '"+std::string(filename)+"'";
	}

	if( Source==_type )
	{
		// count number of columns and rows
		this->cols=convert_count_cols(this->file,this->format.delimiter);
		this->rows=convert_count_rows(this->file);
	}

	// store type and name
	this->type=_type;
	this->name=std::string(varname);
}

void FileASCII::initialize(const Format *source)
{
	if( Destination==this->type )
	{
		// get number of columns and rows
		this->cols=source->getCols();
		this->rows=source->getRows();
	}
	else
	{
		throw std::string("cannot set dimensions to a source file");
	}
}

float FileASCII::getValue(size_t _row, size_t _col)
{
	static size_t lastrow=0;
	static size_t lastcol=0;
	static std::stringstream ss;

	if( lastrow!=_row || lastcol!=_col )
	{
		throw std::string("text file must be accessed sequentially");
	}

	if( !_col )
	{
		// read a data line (or row)
		std::string line;
		if( !std::getline(this->file,line) )
		{
			throw std::string("there are no more values to read");
		}

		// (re)initialize the string stream
		ss.clear();
		ss.str(line);
	}

	// read value
	std::string value;
	if( !std::getline(ss,value,this->format.delimiter) )
	{
		throw std::string("cannot read the requested value");
	}

	// increment columns and rows counters
	lastcol=(lastcol+1<this->cols)? (lastcol+1) : 0;
	if( !lastcol )
	{
		lastrow=(lastrow+1<this->rows)? (lastrow+1) : 0;
	}

	return strtof(value.c_str(),NULL);
}

void FileASCII::setValue(size_t _row, size_t _col, float value)
{
	static size_t lastrow=0;
	static size_t lastcol=0;

	if( lastrow!=_row || lastcol!=_col )
	{
		throw std::string("text file must be accessed sequentially");
	}

	// write value
	this->file << value;

	// write delimiter
	if( this->cols-1>_col )
	{
		this->file << this->format.delimiter;
	}
	else
	{
		this->file << '\n';
	}

	// any problem during write?
	if( this->file.fail() )
	{
		throw std::string("cannot write the requested value");
	}

	// increment columns and rows counters
	lastcol=(lastcol+1<this->cols)? (lastcol+1) : 0;
	if( !lastcol )
	{
		lastrow=(lastrow+1<this->rows)? (lastrow+1) : 0;
	}
}

void FileASCII::loadFormats(void)
{
	FormatData txt={' ',"txt"};
	FormatData csv={',',"csv"};
	FormatData tsv={'\t',"tsv"};

	this->formatlist.push_back(txt);
	this->formatlist.push_back(csv);
	this->formatlist.push_back(tsv);
}
