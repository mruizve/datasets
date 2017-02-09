#ifndef __FACEDATA_CONVERSIONS_H__
#define __FACEDATA_CONVERSIONS_H__

#include<stddef.h>
#include<string>
#include<vector>

typedef enum
{
	Source=0,
	Destination
} IOType;

typedef struct
{
	char delimiter;
	char extension[4];
} FormatData;

typedef std::vector<FormatData> FormatDataArray;

class Format
{
	public:
		// to properly call the close() function
		virtual ~Format(void){};

		// data storage abstraction
		virtual void open(const char *filename, const char *varname, IOType type)=0;
		virtual void initialize(const Format *source)=0;

		// data abstraction
		size_t getCols(void) const;
		size_t getRows(void) const;
		virtual float getValue(size_t row, size_t col)=0;
		virtual void setValue(size_t row, size_t col, float value)=0;

	protected:
		// standard test
		bool tryOpen(const char *filename);

		// manually defined formats
		virtual void loadFormats(void)=0;

		// common data member
		size_t cols;
		size_t rows;
		size_t index;
		IOType type;
		std::string name;
		FormatData format;
		FormatDataArray formatlist;
};

Format* tryOpen(const char *filename, const char *varname, const IOType type);

#endif
