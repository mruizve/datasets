#include<algorithm>        // std::count()
#include<sstream>          // string stream
#include "io/ascii.h"

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

FileASCII::FileASCII(const char *_filename, const char *_varname, IOType _type)
:
	IOFile(_filename,_varname,_type)
{
	// generate list of supported file formats
	IOFileData txt={IOColMajor,' ',"txt"};
	IOFileData csv={IOColMajor,',',"csv"};
	IOFileData tsv={IOColMajor,'\t',"tsv"};
	IOFileDataArray formatlist;
	formatlist.push_back(txt);
	formatlist.push_back(csv);
	formatlist.push_back(tsv);

	// load data or prepare for writing
	if( this->tryOpen(formatlist) )
	{
		this->open();
	}
	else
	{
		throw "'"+this->filename+"' is not a valid ASCII file";
	}
}


FileASCII::~FileASCII(void)
{
	// write output data?
	if( IODestination==this->type )
	{
		// for each data row,
		for( int row=0; this->rows>row; row++ )
		{
			// for each column value (except the last),
			for( int col=0; this->cols-1>col; col++ )
			{
				// write values followed by the data format delimiter
				this->file << *(this->data+col*this->rows+row) << this->format.delimiter;
			}

			// write last column value followed by a new line
			this->file << *(this->data+(this->cols-1)*this->rows+row) << '\n';
		}
	}
	
	// close the file stream
	this->file.close();

	// release memory resources
	delete this->data;
}

void FileASCII::open(void)
{
	// set open mode flags
	std::ios::openmode flags=std::ios::in;
	if( IODestination==this->type )
	{
		flags=(std::ios::out|std::ios::trunc);
	}

	// open the file stream
	this->file.open(this->filename.c_str(),flags);
	if( this->file.fail() )
	{
		throw "cannot open the file '"+std::string(filename)+"'";
	}

	if( IOSource==this->type )
	{
		// count number of columns and rows
		this->cols=convert_count_cols(this->file,this->format.delimiter);
		this->rows=convert_count_rows(this->file);

		// allocate memory resources
		this->data=new float[this->cols*this->rows];

		// load data
		std::string line;
		for( int row=0; std::getline(this->file,line); row++ )
		{
			std::string value;
			std::stringstream ss(line);
			for( int col=0; std::getline(ss,value,this->format.delimiter); col++ )
			{
				*(this->data+col*this->rows+row)=strtof(value.c_str(),NULL);
			}
		}
	}
}

void FileASCII::initialize(const IOFile *source)
{
	if( IODestination==this->type )
	{
		// get number of columns and rows
		this->cols=source->getCols();
		this->rows=source->getRows();

		// allocate memory resources
		this->data=new float[this->cols*this->rows];
	}
	else
	{
		throw std::string("cannot initialize a source file");
	}
}

float FileASCII::getValue(size_t row, size_t col)
{
	if( row>=this->rows || col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	return *(this->data+col*this->rows+row);
}

void FileASCII::setValue(size_t row, size_t col, float value)
{
	if( row>=this->rows || col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	*(this->data+col*this->rows+row)=value;
}

float* FileASCII::getDataPtr(void) const
{
	return this->data;
}
