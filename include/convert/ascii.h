#ifndef __FACEDATA_CONVERSIONS_ASCII_H__
#define __FACEDATA_CONVERSIONS_ASCII_H__

#include<fstream>
#include "convert/format.h"

class FileASCII: public Format
{
	public:
		FileASCII(const char *filename, const char *varname, IOType type);
		~FileASCII(void);

		void open(const char *filename, const char *varname, IOType type);
		void initialize(const Format* source);

		float getValue(size_t row, size_t col);
		void setValue(size_t row, size_t col, float value);

		void loadFormats(void);

	protected:
		std::fstream file;
};

#endif
