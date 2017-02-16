#include "io/mat.h"

FileMAT::FileMAT(const char *_filename, const char *_varname, IOType _type)
:
	IOFile(_filename,_varname,_type)
{
	// generate list of supported file formats
	IOFileData mat={IOColMajor,0,"mat"};
	IOFileDataArray formatlist;
	formatlist.push_back(mat);

	// load data or prepare for writing
	if( this->tryOpen(formatlist) )
	{
		this->open();
	}
	else
	{
		throw "'"+this->filename+"' is not a valid MATLAB file";
	}
}

FileMAT::~FileMAT(void)
{
	if( IODestination==this->type )
	{
		// write data to the output storage file
		if( matPutVariable(this->file,this->varname.c_str(),this->data) )
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

void FileMAT::open(void)
{
	// validate varname
	if( this->varname.empty() )
	{
		throw std::string("missing name of the data container or variable");
	}

	// open the MATLAB file
	if( IOSource==this->type )
	{
		this->file=matOpen(this->filename.c_str(),"r");
		if( NULL==this->file )
		{
			throw "cannot open the input file '"+this->filename+"'";
		}

		// read data format
		this->data=matGetVariable(this->file,this->varname.c_str());
		if( NULL==this->data || 2!=mxGetNumberOfDimensions(this->data) || mxSINGLE_CLASS!=mxGetClassID(this->data) || mxIsComplex(this->data) )
		{
			// empty or invalid data (we expect a two dimensional real single precision -float- matrix)
			throw "cannot read data from '"+this->filename+"'";
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
		this->file=matOpen(this->filename.c_str(),"w");
		if( NULL==this->file )
		{
			throw "cannot open the output file '"+this->filename+"'";
		}
	}
}

void FileMAT::initialize(const IOFile* source)
{
	if( IODestination==this->type )
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
		throw std::string("cannot initialize a source file");
	}
}

float FileMAT::getValue(size_t row, size_t col)
{
	if( row>=this->rows || col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	return *(this->raw+col*this->rows+row);
}

void FileMAT::setValue(size_t row, size_t col, float value)
{
	if( row>=this->rows || col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	*(this->raw+col*this->rows+row)=value;
}

float* FileMAT::getDataPtr(void) const
{
	return this->raw;
}
