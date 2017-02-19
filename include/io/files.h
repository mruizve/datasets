#ifndef __FACEDATA_IOFILE_H__
#define __FACEDATA_IOFILE_H__

#include<stddef.h>
#include<string>
#include<vector>

#ifdef DEBUGGING
	#include<iostream>
	#include<typeinfo>
#endif

typedef enum
{
	IOColMajor=0,
	IORowMajor
} IOMajor;

typedef enum
{
	IOSource=0,
	IODestination
} IOType;

typedef struct
{
	IOMajor ordering;
	char delimiter;
	char extension[4];
} IOFileData;

typedef std::vector<IOFileData> IOFileDataArray;

class IOFile
{
	public:
		// to properly call the close() function
		IOFile(const char *filename, const char *varname, IOType type);
		virtual ~IOFile(void);

		// data storage abstraction
		virtual void open(void)=0;
		virtual void initialize(const IOFile *source)=0;

		// data abstraction
		size_t getCols(void) const;
		size_t getRows(void) const;
		size_t getBytes(void) const;
		IOMajor getMajorOrdering(void) const;

		virtual float getValue(size_t row, size_t col)=0;
		virtual void setValue(size_t row, size_t col, float value)=0;
		virtual float* getDataPtr(void) const =0;

	protected:
		// standard test
		bool tryOpen(IOFileDataArray formatlist);

		// common data member
		int ordering;
		size_t cols;
		size_t rows;
		size_t bytes;
		IOType type;
		std::string varname;
		std::string filename;
		IOFileData format;
};

IOFile* FileOpen(const char *filename, const char *varname, const IOType type);

#endif
