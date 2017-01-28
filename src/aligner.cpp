#include<stdio.h>
#include<string>
#include<opencv2/opencv.hpp>

// extract coordinates values from a 'delimiter'-separated list
template<typename T> T getCoordinates(std::stringstream& ss, const char delimiter)
{
	T output;
    std::string token;

	// first value
    std::getline(ss,token,delimiter);
    output.x=strtof(token.c_str(),NULL);

	// second value
    std::getline(ss,token,delimiter);
    output.y=strtof(token.c_str(),NULL);

    return output;
}

// template wrapper: integer coordinates
cv::Point getCoordinates2i(std::stringstream& ss, const char delimiter)
{
	return getCoordinates<cv::Point>(ss,delimiter);
}

// template wrapper: floating coordinates
cv::Point2f getCoordinates2f(std::stringstream& ss, const char delimiter)
{
	return getCoordinates<cv::Point2f>(ss,delimiter);
}

int main(int argc, char *argv[])
{
	// validate input arguments
	if( 5!=argc )
	{
		fprintf(stderr,
			"\nusage: %s source landmarks dimensions destination\n\n"
			"   source: input image\n"
			"   landmarks: comma separated coordinates of the eyes (center), nose and mouth\n"
			"   dimensions: output dimensions (e.g., 224x224) \n"
			"   destination: output image\n\n",
			argv[0]);

		return 1;
	}

	// load input image
	cv::Mat input=cv::imread(argv[1]);
	if( NULL==input.data )
	{
		fprintf(stderr,"(EE) %s: cannot load the input image\n     >> input: '%s'\n",argv[0],argv[1]);
		return 1;
	}

	// extract landmarks
	std::stringstream ss(argv[2]);
	cv::Point2f le=getCoordinates2f(ss,','); // left eye
	cv::Point2f re=getCoordinates2f(ss,','); // right eye
	cv::Point2f n=getCoordinates2f(ss,',');  // nose
	cv::Point2f m=getCoordinates2f(ss,',');  // mouth

	#ifdef DEBUGGING
		// draw landmarks
		cv::line(input,le,re,cv::Scalar(255,255,255),3);
		cv::line(input,le,n,cv::Scalar(255,255,255),3);
		cv::line(input,re,n,cv::Scalar(255,255,255),3);
		cv::line(input,n,m,cv::Scalar(255,255,255),3);
		cv::circle(input,le,7,cv::Scalar(255,0,0),-1);
		cv::circle(input,re,7,cv::Scalar(255,0,255),-1);
		cv::circle(input,n,7,cv::Scalar(0,0,255),-1);
		cv::circle(input,m,7,cv::Scalar(0,255,255),-1);
	#endif

	// extract output dimensions
	ss.str(argv[3]);
	cv::Point dimension=getCoordinates2i(ss,'x'); // x: width, y: height

	cv::Point d=le-re;
	if( sqrt(d.x*d.x+d.y*d.y)<30 )
	{
		fprintf(stderr,"%s --> cazzo!\n",argv[1]);
	} 

	#ifdef DEBUGGING
		cv::namedWindow("debug",CV_WINDOW_NORMAL);
		cv::imshow("debug",input);
		cv::waitKey(100);
	#endif

	return 0;
}
