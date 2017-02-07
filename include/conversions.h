#ifndef __FACEDATA_CONVERSIONS_H__
#define __FACEDATA_CONVERSIONS_H__

#define ASCII 0
#define CSV   1
#define TSV   2
#define XML   3
#define DAT   4
#define MAT   5

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
