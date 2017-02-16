#ifndef __FACEDATA_IOFILE_MAT_H__
#define __FACEDATA_IOFILE_MAT_H__

#include<mat.h>             // MATLAB
#include "io/files.h"

class FileMAT: public IOFile
{
	public:
		FileMAT(const char *filename, const char *varname, IOType type);
		~FileMAT(void);

		void open(void);
		void initialize(const IOFile *source);

		float getValue(size_t row, size_t col);
		void setValue(size_t row, size_t col, float value);
		float* getDataPtr(void) const;

	protected:
		float *raw;
		mxArray *data;
		MATFile *file;
};

#endif
