#include<string.h>
#include "conversions.h"

const convert_t FORMATS[]=
{
	{ASCII,' ',"txt",NULL},   // ASCII values separated with spaces
	{CSV,',',"csv",NULL},   // ASCII values separated with comma 
	{TSV,'\t',"tsv",NULL},  // ASCII values separated with tabs 
	{XML,0,"xml",NULL},     // OpenCV XML format
	{DAT,0,"dat",NULL},     // tSNE .dat format
	{MAT,0,"mat",NULL},     // MATLAB .mat format
	{-1,-1,"",NULL}       // empty (returned on error)
};

const convert_t convert_get_format(char *filename)
{
	// validate input arguments
	if( NULL==filename )
	{
		throw "filename cannot be NULL";
	}

	char *dot=strrchr(filename,'.')+1;
	char *slash=strrchr(filename,'/');

	// default format
	convert_t format={-1,-1,"",NULL};
	(void)memcpy(&format,&FORMATS[0],sizeof(convert_t));

	// if the given path has a valid (three characters long) extension, then
	if( slash<=dot && (filename+strlen(filename)-3)==dot )
	{
		// loop through all the formats list (except the first)
		for( int i=1; sizeof(FORMATS)/sizeof(convert_t)-1>i; i++ )
		{
			// if the path extension corresponds to one of the list, then
			if( !strcasecmp(dot,(FORMATS+i)->extension) )
			{
				// set new format and break the loop
				(void)memcpy(&format,&FORMATS[i],sizeof(convert_t));
				break;
			}
		}
	}

	// set path for conversion
	format.path=filename;

	return format;
}
