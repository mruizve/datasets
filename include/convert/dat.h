#ifndef __FACEDATA_CONVERSIONS_DAT_H__
#define __FACEDATA_CONVERSIONS_DAT_H__

#include "convert/format.h"

class FileDAT: public Format
{
	public:
		FileDAT(const char *filename, const char *varname);
		~FileDAT(void);

		void open(const char *filename, const char *varname);
		void initialize(const Format *source);

		float getValue(size_t row, size_t col);
		void setValue(size_t row, size_t col, float value);
		void loadFormats(void);

	protected:
};

#endif
