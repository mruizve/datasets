// C headers
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>

// C++ headers
#include<fstream>
#include<string>
#include<opencv2/opencv.hpp>

// extract values from a list
std::vector<cv::Point2f> getCoordinates(const std::string list, const int n, const char delimiter)
{
	std::stringstream ss(list);
	std::vector<cv::Point2f> p;

	// extract n coordinate pairs
	for( int i=0; n>i; i++ )
	{
		std::string strx,stry;
		std::getline(ss,strx,delimiter);
		std::getline(ss,stry,delimiter);

		float x=strtof(strx.c_str(),NULL);
		float y=strtof(stry.c_str(),NULL);
		p.push_back(cv::Point2f(x,y));
	}

    return p;
}

int main(int argc, char *argv[])
{
	int err;
	
	// validate input arguments
	if( 4!=argc )
	{
		fprintf(stderr,
			"\nusage: %s dataset association dimension\n\n"
			"   dataset: full path to the dataset root directory\n"
			"   association: association file generated using the evaluation scripts\n"
			"   dimension: dimensions of the output images (e.g., 224x224) \n",
			argv[0]);

		return -1;
	}

	// validate dataset directory
    struct stat stats;
    std::string dataset(argv[1]); 
    err=stat(argv[1],&stats);
    if( 0>err )
    {
		fprintf(stderr,"\n(EE) %s: invalid dataset directory\n(EE) >>> dataset: '%s'",argv[0],argv[1]);
		return -1;
	}

	// open the association file
	std::string fullpath(std::string(argv[1])+"/"+argv[2]);
	std::ifstream association(fullpath.c_str());
	if( association.fail() )
	{
		fprintf(stderr,"\n(EE) %s: invalid association file\n(EE) >>> association: '%s'",argv[0],fullpath.c_str());
		return -1;
	}

	// get output images dimensions
	std::vector<cv::Point2f> dims=getCoordinates(argv[3],1,'x');

	#ifdef DEBUGGING
		int verbose=1;
		int olderr,newerr;
		std::string iwname("input -- press ESC key to stop preview");
		std::string owname("output -- press ESC key to stop preview");

		// redirection of stderr to suppress OpenCV/QT useless info
		fflush(stderr);
		olderr=dup(STDERR_FILENO);
		newerr=open("/dev/null",O_WRONLY);
		dup2(newerr,STDERR_FILENO);
		close(newerr);
			cv::namedWindow(iwname,CV_WINDOW_NORMAL);
			cv::namedWindow(owname.at(1),CV_WINDOW_NORMAL);
		fflush(stderr);
		dup2(olderr,STDERR_FILENO);
		close(olderr);

		// wait some time to avoid segmentation fault when early errors occurs
		sleep(1);
	#endif

	// image processing loop
	std::string line;
	std::string ipath,label,opath,strbbox,strmarks;
	int processed=0;
	while( std::getline(association,line) )
	{
		// parse line
		std::stringstream ss(line);
		std::getline(ss,opath,' ');
		std::getline(ss,label,' ');
		std::getline(ss,ipath,' ');
		std::getline(ss,strbbox,' ');
		std::getline(ss,strmarks,' ');
		
		// load the input image
		ipath=dataset+"/"+ipath;
		cv::Mat input=cv::imread(ipath);
		if( NULL==input.data )
		{
			fprintf(stderr,"\n(EE) %s: cannot load image\n(EE) >>> image: '%s'",argv[0],ipath.c_str());
			return -1;
		}

		// extract bounding boxes
		std::vector<cv::Point2f> bb=getCoordinates(strbbox,2,','); // {(left,top),(width,height)}

		// extract landmarks
		std::vector<cv::Point2f> lm=getCoordinates(strmarks,4,','); // {(left eye),(right eye),(nose),(mouth)}

		// output landmarks locations
		cv::Point2f eyes=lm[0]-lm[1];
		cv::Point2f nose=lm[2]-lm[1];
		cv::Point2f center=0.5*dims[0];

		float ho=35.0f; // horizontal offset
		float vo=35.0f; // vertical offset
		float vs=0.8f;  // vertical shift

		std::vector<cv::Point2f> dst;
		dst.push_back(center+cv::Point2f(-ho,    -vs*vo));
		dst.push_back(center+cv::Point2f( ho,    -vs*vo));
		dst.push_back(center+cv::Point2f(  0, (2-vs)*vo));
		
		// input landmarks locations
		std::vector<cv::Point2f> src;
		src.push_back(lm[0]);
		src.push_back(lm[1]);
		src.push_back(lm[3]);

		// compute perspective transform
		cv::Mat H=cv::getAffineTransform(src,dst);

		// align face used the above computed transform
		cv::Mat output;
		cv::warpAffine(input,output,H,cv::Size(dims[0].x,dims[0].y),cv::INTER_CUBIC);

		// save output image
		opath=dataset+"/"+opath;
		cv::imwrite(opath,output);

		processed++;
		printf("%06d\b\b\b\b\b\b",processed);
		fflush(stdout);

		#ifdef DEBUGGING
			if( verbose )
			{
				// draw bounding box
				cv::rectangle(input,bb[0],bb[0]+bb[1],cv::Scalar(0,255,0),3);

				// draw landmarks
				cv::line(input,lm[0],lm[1],cv::Scalar(255,255,255),3);
				cv::line(input,lm[0],lm[2],cv::Scalar(255,255,255),3);
				cv::line(input,lm[1],lm[2],cv::Scalar(255,255,255),3);
				cv::line(input,lm[2],lm[3],cv::Scalar(255,255,255),3);
				cv::circle(input,lm[0],7,cv::Scalar(255,0,0),-1);
				cv::circle(input,lm[1],7,cv::Scalar(255,0,255),-1);
				cv::circle(input,lm[2],7,cv::Scalar(0,0,255),-1);
				cv::circle(input,lm[3],7,cv::Scalar(0,255,255),-1);

				// show input and output images 
				cv::imshow(iwname,input);
				cv::imshow(owname,output);
				int key=cv::waitKey(500);

				// update verbose flags
				verbose=(key!=0x1b);
				if( !verbose )
				{
					cv::destroyWindow(iwname);
					cv::destroyWindow(owname);
				}
			}
		#endif
	}

	return 0;
}
