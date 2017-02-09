#ifndef __FACEDATA_CONVERSIONS_MAT_H__
#define __FACEDATA_CONVERSIONS_MAT_H__

#include<mat.h>             // MATLAB
#include "convert/format.h"

class FileMAT: public Format
{
	public:
		FileMAT(const char *filename, const char *varname, IOType type);
		~FileMAT(void);

		void open(const char *filename, const char *varname, IOType type);
		void initialize(const Format *source);

		float getValue(size_t row, size_t col);
		void setValue(size_t row, size_t col, float value);

		void loadFormats(void);

	protected:
		float *raw;
		mxArray *data;
		MATFile *file;
};

#endif
