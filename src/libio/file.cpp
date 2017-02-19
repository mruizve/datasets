#include<string.h>        // strcasecmp()
#include<sys/stat.h>      // struct stat, stat(), S_ISREG()
#include<string>
#include "io/ascii.h"
#include "io/mat.h"
#include "io/xml.h"

IOFile::IOFile(const char *_filename, const char *_varname, IOType _type)
{
	// validate filename
	try
	{
		this->filename=std::string(_filename);
		if( this->filename.empty() )
		{
			throw;
		}
	}
	catch( ... )
	{
		throw std::string("filename cannot be NULL or empty");
	}

	// validate varname
	// (we allow any value, since depends on the IOFile format)
	this->varname=(NULL!=_varname)? std::string(_varname) : std::string() ;

	this->type=_type;

	#ifdef DEBUGGING
		std::cout << "(DD) created " << typeid(this).name() << std::endl;
	#endif
}

IOFile::~IOFile(void)
{
	#ifdef DEBUGGING
		std::cout << "(DD) deleted " << typeid(this).name() << std::endl;
	#endif
}

size_t IOFile::getCols(void) const
{
	return this->cols;
}

size_t IOFile::getRows(void) const
{
	return this->rows;
}

size_t IOFile::getBytes(void) const
{
	return this->bytes;
}

IOMajor IOFile::getMajorOrdering(void) const
{
	return this->format.ordering;
}

bool IOFile::tryOpen(IOFileDataArray formatlist)
{
	const char *fname=this->filename.c_str();
	const char *dot=strrchr(fname,'.')+1;
	const char *slash=strrchr(fname,'/');

	// if the given path has a valid (three characters long) extension, then
	if( slash<=dot && (fname+strlen(fname)-3)==dot )
	{
		// loop through all the formats list
		for( int i=0; formatlist.size()>i; i++ )
		{
			// if the path extension corresponds to one of the list, then
			if( !strcasecmp(dot,formatlist[i].extension) )
			{
				// format found
				this->format=formatlist[i];
				return 1;
			}
		}
	}

	this->format=IOFileData();
	return 0;
}

template<typename T> static IOFile* __FileOpen(const char *filename, const char *varname, const IOType type)
{
	try
	{
		return new T(filename,varname,type);
	}
	#ifdef DEBUGGING
		catch( const std::string& error )
		{
			std::cerr << "(EE) [" << __FILE__ << ":" << __LINE__ << "] " << error << std::endl;
		}
	#else
		catch( ... ){}
	#endif

	return NULL;
}

IOFile* FileOpen(const char *filename, const char *varname, const IOType type)
{
	// validate paths
	struct stat stats;
	if( IOSource==type && (0>stat(filename,&stats) || !S_ISREG(stats.st_mode)) )
	{
		throw "invalid input file '"+std::string(filename)+"'";
	}

	if( IODestination==type && !stat(filename,&stats) )
	{
		// path exists, it is a regular file?
		if( !S_ISREG(stats.st_mode) )
		{
			// no, it is not a regular file
			throw "invalid output file '"+std::string(filename)+"'";
		}

		std::string line;
		do
		{
			std::cout << "(\?\?) overwrite the output file '" << filename << "'? ";
		}
		while( std::getline(std::cin,line) && line.compare("n") && line.compare("y") );

		// if not, abort conversion
		if( line.compare("y") )
		{
			throw std::string("conversion aborted as requested");
		}
	}

	IOFile *file;

	file=__FileOpen<FileASCII>(filename,varname,type);
	if( NULL!=file )
	{
		return file;
	}

	//file=__FileOpen<FileDAT>(filename,varname,type);
	//if( NULL!=file )
	//{
	//	return file;
	//}

	file=__FileOpen<FileMAT>(filename,varname,type);
	if( NULL!=file )
	{
		return file;
	}

	file=__FileOpen<FileXML>(filename,varname,type);
	if( NULL!=file )
	{
		return file;
	}

	return NULL;
}
