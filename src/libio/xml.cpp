#include "io/xml.h"

FileXML::FileXML(const char *_filename, const char *_varname, IOType _type)
:
	IOFile(_filename,_varname,_type)
{
	// generate list of supported file formats
	IOFileData xml={IORowMajor,0,"xml"};
	IOFileDataArray formatlist;
	formatlist.push_back(xml);

	// load data or prepare for writing
	if( this->tryOpen(formatlist) )
	{
		this->open();
	}
	else
	{
		throw "'"+this->filename+"' is not a valid XML file";
	}
}

FileXML::~FileXML(void)
{
	if( IODestination==this->type )
	{
		// write data
		this->file << this->varname << this->data;
	}

	// close the XML file
    this->file.release();
}

void FileXML::open(void)
{
	// validate varname
	if( this->varname.empty() )
	{
		throw std::string("missing name of the data container or variable");
	}

	// open the XML file
	try
	{
		int flags=cv::FileStorage::READ;
		if( IODestination==this->type )
		{
			flags=cv::FileStorage::WRITE;
		}
		this->file.open(this->filename,flags);
	}
	catch( const cv::Exception& error )
	{
		throw std::string("unexpected exception from cv::FileStorage::open()");
	}

	if( !this->file.isOpened() )
	{
		throw "cannot open the file '"+this->filename+"'";
	}

	// read data?
	if( IOSource==this->type )
	{
		this->file[this->varname] >> this->data;
		if( this->data.empty() || 1!=this->data.channels() )
		{
			// empty or invalid data (we expect single channel matrices)
			throw "cannot read data from '"+this->filename+"'";
		}

		// get number of columns and rows
		this->cols=this->data.cols;
		this->rows=this->data.rows;
	}
}

void FileXML::initialize(const IOFile* source)
{
	if( IODestination==this->type )
	{
		// allocate data
		this->data=cv::Mat(source->getRows(),source->getCols(),CV_32FC1);

		// get number of columns and rows
		this->cols=this->data.cols;
		this->rows=this->data.rows;
	}
	else
	{
		throw std::string("cannot initialize a source file");
	}
}

float FileXML::getValue(size_t row, size_t col)
{
	if( row>=this->rows || col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	return this->data.at<float>(row,col);
}

void FileXML::setValue(size_t row, size_t col, float value)
{
	if( row>=this->rows || col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	this->data.at<float>(row,col)=value;
}

float* FileXML::getDataPtr(void) const
{
	return (float*)this->data.data;
}
