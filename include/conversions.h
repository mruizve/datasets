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

#define MY_ASCII 0
#define MY_XML   1
#define MY_DAT   2
#define MY_MAT   3

#define CONVERSION(input,output) (((input)<<8)|(output))

typedef struct
{
	const int id;
	const int delimiter;
	const char extension[4];
	char* path;
} convert_t;

// available list of formats, should be defined on src/convert/formats.cpp
extern const convert_t FORMATS[];

// get data format based on the path extension
const convert_t convert_get_format(char *filename);

// conversion from text
void convert_text2text(const convert_t input, const convert_t output);
void convert_text2xml(const convert_t input, const convert_t output, const char *varname);
void convert_text2dat(const convert_t input, const convert_t output, const char *varname);
void convert_text2mat(const convert_t input, const convert_t output, const char *varname);

// conversions from XML
void convert_xml2text(const convert_t input, const convert_t output, const char *varname);
void convert_xml2xml(const convert_t input, const convert_t output, const char *varname);
void convert_xml2dat(const convert_t input, const convert_t output, const char *varname);
void convert_xml2mat(const convert_t input, const convert_t output, const char *varname);

// conversions from .dat
void convert_dat2text(const convert_t input, const convert_t output, const char *varname);
void convert_dat2xml(const convert_t input, const convert_t output, const char *varname);
void convert_dat2dat(const convert_t input, const convert_t output, const char *varname);
void convert_dat2mat(const convert_t input, const convert_t output, const char *varname);

// conversions from .mat
void convert_mat2text(const convert_t input, const convert_t output, const char *varname);
void convert_mat2xml(const convert_t input, const convert_t output, const char *varname);
void convert_mat2dat(const convert_t input, const convert_t output, const char *varname);
void convert_mat2mat(const convert_t input, const convert_t output, const char *varname);

#endif
