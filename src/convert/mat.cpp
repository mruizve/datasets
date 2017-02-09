#include "convert/mat.h"

FileMAT::FileMAT(const char *filename, const char *varname, IOType type)
{
	// load supported formats
	this->loadFormats();

	// prepare for data reading or writing
	this->open(filename,varname,type);
}

FileMAT::~FileMAT(void)
{
	if( Destination==this->type )
	{
		// write data to the output storage file
		if( matPutVariable(this->file,this->name.c_str(),this->data) )
		{
			throw std::string("unexpected error from matPutVariable()");
		}
	}

	// release data
	mxDestroyArray(this->data);

	// close the storage file
	if( matClose(this->file) )
	{
		throw std::string("unexpected error from matClose()");
	}
}

void FileMAT::open(const char *filename, const char *varname, IOType _type)
{
	if( !this->tryOpen(filename) )
	{
		throw "'"+std::string(filename)+"' is not a valid XML file";
	}

	// open the MATLAB file
	if( Source==_type )
	{
		this->file=matOpen(filename,"r");
		if( NULL==this->file )
		{
			throw "cannot open the input file '"+std::string(filename)+"'";
		}

		// read data format
		this->data=matGetVariable(file,varname);
		if( NULL==this->data || 2!=mxGetNumberOfDimensions(this->data) || mxSINGLE_CLASS!=mxGetClassID(this->data) || mxIsComplex(this->data) )
		{
			// empty or invalid data (we expect a two dimensional real single precision -float- matrix)
			throw "cannot read data from '"+std::string(filename)+"'";
		}

		// get a pointer to the data buffer
		this->raw=(float*)mxGetData(this->data);
		if( NULL==this->raw )
		{
			throw std::string("unexpected error from mxGetData()'");
		}

		// get number of columns and rows
		this->cols=mxGetN(this->data);
		this->rows=mxGetM(this->data);
	}
	else
	{
		this->file=matOpen(filename,"w");
		if( NULL==this->file )
		{
			throw "cannot open the output file '"+std::string(filename)+"'";
		}
	}

	// store type and name
	this->type=_type;
	this->name=std::string(varname);
}

void FileMAT::initialize(const Format* source)
{
	if( Destination==this->type )
	{
		// initialize data structure
		this->data=mxCreateNumericMatrix(source->getRows(),source->getCols(),mxSINGLE_CLASS,mxREAL);
		if( NULL==data )
		{
			throw std::string("unexpected error from mxCreateNumericMatrix()'");
		}

		// get a pointer to the data buffer
		this->raw=(float*)mxGetData(this->data);
		if( NULL==this->raw )
		{
			throw std::string("unexpected error from mxGetData()'");
		}

		// get number of columns and rows
		this->cols=mxGetN(this->data);
		this->rows=mxGetM(this->data);
	}
	else
	{
		throw std::string("cannot set dimensions to a source file");
	}
}

float FileMAT::getValue(size_t _row, size_t _col)
{
	if( _row>=this->rows || _col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	return *(this->raw+_col*this->rows+_row);
}

void FileMAT::setValue(size_t _row, size_t _col, float value)
{
	if( _row>=this->rows || _col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	*(this->raw+_col*this->rows+_row)=value;
}

void FileMAT::loadFormats(void)
{
	FormatData mat={0,"mat"};

	this->formatlist.push_back(mat);
}
