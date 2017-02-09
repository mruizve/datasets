#include<string.h>        // strcasecmp()
#include<string>
#include "convert/ascii.h"
#include "convert/mat.h"
#include "convert/xml.h"

size_t Format::getCols(void) const
{
	return this->cols;
}

size_t Format::getRows(void) const
{
	return this->rows;
}

bool Format::tryOpen(const char *filename)
{
	// validate input arguments
	if( NULL==filename )
	{
		throw std::string("filename cannot be NULL");
	}

	const char *dot=strrchr(filename,'.')+1;
	const char *slash=strrchr(filename,'/');

	// if the given path has a valid (three characters long) extension, then
	if( slash<=dot && (filename+strlen(filename)-3)==dot )
	{
		// loop through all the formats list
		for( int i=0; this->formatlist.size()>i; i++ )
		{
			// if the path extension corresponds to one of the list, then
			if( !strcasecmp(dot,this->formatlist[i].extension) )
			{
				// format found, let's try open the file
				this->index=i;
				this->format=this->formatlist[i];
				return 1;
			}
		}
	}

	this->index=-1;
	this->format=FormatData();
	return 0;
}

Format* tryOpen(const char *filename, const char *varname, const IOType type)
{
	try
	{
		return new FileASCII(filename,varname,type);
	}
	catch( ... ){}

	// try
	// {
		// return new FileDAT(filename,varname,type);
	// }
	// catch( ... ){}

	try
	{
		return new FileMAT(filename,varname,type);
	}
	catch( ... ){}

	try
	{
		return new FileXML(filename,varname,type);
	}
	catch( ... ){}

	return NULL;
}
