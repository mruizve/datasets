#ifndef __FACEDATA_IOFILE_XML_H__
#define __FACEDATA_IOFILE_XML_H__

#include<opencv2/opencv.hpp>
#include "io/files.h"

class FileXML: public IOFile
{
	public:
		FileXML(const char *filename, const char *varname, IOType type);
		~FileXML(void);

		void open(void);
		void initialize(const IOFile *source);

		float getValue(size_t row, size_t col);
		void setValue(size_t row, size_t col, float value);
		float* getDataPtr(void) const;

	protected:
		cv::Mat data;
		cv::FileStorage file;
};

#endif
