#ifndef __FACEDATA_CONVERSIONS_XML_H__
#define __FACEDATA_CONVERSIONS_XML_H__

#include "convert/format.h"
#include<opencv2/opencv.hpp>

class FileXML: public Format
{
	public:
		FileXML(const char *filename, const char *varname, IOType type);
		~FileXML(void);

		void open(const char *filename, const char *varname, IOType type);
		void initialize(const Format *source);

		float getValue(size_t row, size_t col);
		void setValue(size_t row, size_t col, float value);

		void loadFormats(void);

	protected:
		cv::Mat data;
		cv::FileStorage file;
};

#endif
