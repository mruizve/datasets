#include "convert/xml.h"

FileXML::FileXML(const char *filename, const char *varname, IOType type)
{
	// load supported formats
	this->loadFormats();

	// prepare for data reading or writing
	this->open(filename,varname,type);
}

FileXML::~FileXML(void)
{
	if( Destination==this->type )
	{
		// write data
		this->file << this->name << this->data;
	}

	// close the XML file
    this->file.release();
}

void FileXML::open(const char *filename, const char *varname, IOType _type)
{
	if( !this->tryOpen(filename) )
	{
		throw "'"+std::string(filename)+"' is not a valid XML file";
	}

	// open the XML file
	try
	{
		int flags=(Source==_type)?(cv::FileStorage::READ):(cv::FileStorage::WRITE);
		this->file.open(filename,flags);
	}
	catch( const cv::Exception& error )
	{
		throw std::string("unexpected exception from cv::FileStorage::open()");
	}
	if( !this->file.isOpened() )
	{
		throw "cannot open the file '"+std::string(filename)+"'";
	}

	// read data?
	if( Source==_type )
	{
		this->file[std::string(varname)] >> this->data;
		if( this->data.empty() || 1!=this->data.channels() )
		{
			// empty or invalid data (we expect single channel matrices)
			throw "cannot read data from '"+std::string(filename)+"'";
		}

		// get number of columns and rows
		this->cols=this->data.cols;
		this->rows=this->data.rows;
	}

	// store type and name
	this->type=_type;
	this->name=std::string(varname);
}

void FileXML::initialize(const Format* source)
{
	if( Destination==this->type )
	{
		// allocate data
		this->data=cv::Mat(source->getRows(),source->getCols(),CV_32FC1);

		// get number of columns and rows
		this->cols=this->data.cols;
		this->rows=this->data.rows;
	}
	else
	{
		throw std::string("cannot set dimensions to a source file");
	}
}

float FileXML::getValue(size_t _row, size_t _col)
{
	if( _row>=this->rows || _col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	return this->data.at<float>(_row,_col);
}

void FileXML::setValue(size_t _row, size_t _col, float value)
{
	if( _row>=this->rows || _col>=this->cols )
	{
		throw std::string("requested element outside data bounds");
	}

	this->data.at<float>(_row,_col)=value;
}

void FileXML::loadFormats(void)
{
	FormatData xml={0,"xml"};

	this->formatlist.push_back(xml);
}
