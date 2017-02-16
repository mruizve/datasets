#ifndef __FACEDATA_IOFILE_DAT_H__
#define __FACEDATA_IOFILE_DAT_H__

#include "io/files.h"

class FileDAT: public IOFile
{
	public:
		FileDAT(const char *filename, const char *varname);
		~FileDAT(void);

		void open(void);
		void initialize(const IOFile *source);

		float getValue(size_t row, size_t col);
		void setValue(size_t row, size_t col, float value);
		float* getDataPtr(void) const;

	protected:
};

#endif
