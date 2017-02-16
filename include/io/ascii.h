#ifndef __FACEDATA_IOFILE_ASCII_H__
#define __FACEDATA_IOFILE_ASCII_H__

#include<fstream>
#include "io/files.h"

class FileASCII: public IOFile
{
	public:
		FileASCII(const char *filename, const char *varname, IOType type);
		~FileASCII(void);

		void open(void);
		void initialize(const IOFile* source);

		float getValue(size_t row, size_t col);
		void setValue(size_t row, size_t col, float value);
		float* getDataPtr(void) const;

	protected:
		float *data;
		std::fstream file;
};

#endif
