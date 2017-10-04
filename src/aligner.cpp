#include<getopt.h>
#include<sys/stat.h>         // struct stat, stat(), S_ISDIR()
#include<fstream>
#include<string>
#include<opencv2/opencv.hpp>
#ifdef DEBUGGING
	#include<fcntl.h>
	#include<unistd.h>
#endif

void usage(const char *argv0)
{
	std::cerr <<  "usage: "<< argv0 <<
		" [options] dataset association dimensions\n\n"
		"       dataset: full path to the dataset root directory\n"
		"   association: output association file\n"
		"    dimensions: dimensions of the output images (e.g., 224x224)\n\n"
		"available options:\n"
		"   -a, --align affine alignment of faces (default: crop, rotate and resize)\n";
}

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
	bool align=0;
	
	// validate input arguments
	static struct option options[] =
	{
		{"align",no_argument,0,'a'},
		{0,0,0,0}
	};

	int idx=0;
	int c=getopt_long(argc,argv,"a",options,&idx);
	for( ; -1!=c; c=getopt_long(argc,argv,"a",options,&idx) )
	{
		switch( c )
		{
			case 'a': align=1; break;
			case '?':
			default:
				usage(argv[0]);
				return -1;
		}
	}

	if( 3!=(argc-optind) )
	{
		usage(argv[0]);
		return -1;
	}

	int err=0;
    std::string dataset(argv[optind]); 
    std::string fullpath(dataset+"/"+argv[optind+1]);
    std::string dimensions(argv[optind+2]); 

	// validate dataset directory
    struct stat stats;
    if( 0>stat(dataset.c_str(),&stats) || !S_ISDIR(stats.st_mode) )
    {
		std::cerr << "(EE) " << argv[0] << ": invalid dataset directory '" << dataset << "'\n";
		return -1;
	}

	// open the association file
	std::ifstream association(fullpath.c_str());
	if( association.fail() )
	{
		std::cerr << "(EE) " << argv[0] << ": cannot create the association file '" << fullpath.c_str() << "'\n";
		return -1;
	}

	// get output images dimensions
	std::vector<cv::Point2f> dims=getCoordinates(dimensions,1,'x');

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
			cv::namedWindow(owname,CV_WINDOW_NORMAL);
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
			std::cerr << "(EE) " << argv[0] << ": cannot load the image '" << ipath.c_str() << "'\n";
			return -1;
		}

		// extract bounding boxes
		std::vector<cv::Point2f> bb=getCoordinates(strbbox,2,','); // {(left,top),(width,height)}

		// rectify bounding box (negative values may imply that the head is turned up-down)
		if( 0.0f>bb.at(1).x )
		{
			bb.at(0).x+=bb.at(1).x;
			bb.at(1).x=-bb.at(1).x;
		}
		if( 0.0f>bb.at(1).y )
		{
			bb.at(0).y+=bb.at(1).y;
			bb.at(1).y=-bb.at(1).y;
		}

		// extract landmarks
		std::vector<cv::Point2f> lm=getCoordinates(strmarks,4,','); // {(left eye),(right eye),(nose),(mouth)}

		// output image
		cv::Mat output;

		// use affine alignment?
		if( align )
		{
			// compute landmark references
			cv::Point2f center=0.5*dims.at(0);

			// this hard coded values should be set as options
			float ho=35.0f; // horizontal offset
			float vo=35.0f; // vertical offset
			float vs=0.8f;  // vertical shift

			// output landmarks locations
			std::vector<cv::Point2f> dst;
			dst.push_back(center+cv::Point2f(-ho,    -vs*vo));
			dst.push_back(center+cv::Point2f( ho,    -vs*vo));
			dst.push_back(center+cv::Point2f(  0, (2-vs)*vo));
			
			// input landmarks locations
			std::vector<cv::Point2f> src;
			src.push_back(lm.at(0));
			src.push_back(lm.at(1));
			src.push_back(lm.at(3));

			// compute perspective transform
			cv::Mat H=cv::getAffineTransform(src,dst);

			// align face used the above computed transform
			cv::warpAffine(input,output,H,cv::Size(dims.at(0).x,dims.at(0).y),cv::INTER_CUBIC);
		}
		else
		{
			// compute maximum distance between bounding box and image borders
			float hratio=bb.at(1).x/dims.at(0).x;
			float vratio=bb.at(1).y/dims.at(0).y;

			if( hratio<=vratio )
			{
				// match output ratio by increasing input width
				float hlength=vratio*dims.at(0).x;

				// if number of input columns < desired bounding box width, then
				if( hlength>input.cols )
				{
					// get the maximum number of columns
					bb.at(0).x=0;
					bb.at(1).x=input.cols;

					// update horizontal ratio
					hratio=bb.at(1).x/dims.at(0).x;

					// crop accordingly the bounding box height
					float vlength=hratio*dims.at(0).y;
					bb.at(0).y=bb.at(0).y-0.5f*(vlength-bb.at(1).y);
					bb.at(1).y=vlength;
				}
				else
				{
					// determine horizontal shift
					float hright=bb.at(0).x+bb.at(1).x;
					float hshift=0.5f*(hlength-bb.at(1).x);

					// crop accordingly the bounding box width
					bb.at(0).x=bb.at(0).x-hshift;
					bb.at(1).x=hlength;

					// rectify left if negative
					if( 0>bb.at(0).x )
					{
						bb.at(0).x=0;
					}

					// shift bounding box to the left?
					if( hshift>(input.cols-hright) )
					{
						bb.at(0).x=bb.at(0).x-hshift+input.cols-hright;
					}
				}
			}
			else
			{
				// match output ratio by increasing input height
				float vlength=hratio*dims.at(0).y;

				// if the number of input rows < desired bounding box height, then
				if( vlength>input.rows )
				{
					// get the maximum number of rows
					bb.at(0).y=0;
					bb.at(1).y=input.rows;

					// update vertical ratio
					vratio=bb.at(1).y/dims.at(0).y;

					// crop accordingly the bounding box width
					float hlength=vratio*dims.at(0).x;
					bb.at(0).x=bb.at(0).x-0.5f*(hlength-bb.at(1).x);
					bb.at(1).x=hlength;
				}
				else
				{
					// determine vertical shift
					float vshift=0.5f*(vlength-bb.at(1).y);
					float hbottom=bb.at(0).y+bb.at(1).y;

					// crop accordingly the bounding box height
					bb.at(0).y=bb.at(0).y-0.5f*(vlength-bb.at(1).y);
					bb.at(1).y=vlength;

					// rectify top if negative
					if( 0>bb.at(0).y )
					{
						bb.at(0).y=0;
					}

					// shift bounding box to the top?
					if( vshift>(input.rows-hbottom) )
					{
						bb.at(0).y=bb.at(0).y-vshift+input.rows-hbottom;
					}
				}
			}

			// crop face region
			cv::Rect roi(bb.at(0).x,bb.at(0).y,bb.at(1).x,bb.at(1).y);
			cv::Mat faceroi;
			input(roi).copyTo(faceroi);

			// get eyes direction
			cv::Point2f orth;
			cv::Point2f eyes=lm.at(0)-lm.at(1);
			eyes=eyes/sqrtf(eyes.x*eyes.x+eyes.y*eyes.y);
			orth=cv::Point2f(eyes.y,-eyes.x);

			// rotate image?
			if( fabsf(eyes.y)>fabs(eyes.x) )
			{
				cv::transpose(faceroi,faceroi);

				// flip image?
				if( 0.0f>orth.x )
				{
					cv::flip(faceroi,faceroi,0);
				}
			}
			else
			{
				// flip image?
				if( 0.0f>orth.y )
				{
					cv::flip(faceroi,faceroi,0);
				}
			}

			// resize the cropped region to meet output dimensions
			cv::resize(faceroi,output,cv::Size(dims.at(0).x,dims.at(0).y),0,0,cv::INTER_LINEAR);
		}

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
				cv::line(input,bb.at(0),bb.at(0)+cv::Point2f(0,bb.at(1).y),cv::Scalar(255,255,255),3);
				cv::line(input,bb.at(0)+cv::Point2f(0,bb.at(1).y),bb.at(0)+bb.at(1),cv::Scalar(255,255,255),3);
				cv::line(input,bb.at(0)+bb.at(1),bb.at(0)+cv::Point2f(bb.at(1).x,0),cv::Scalar(255,255,255),3);
				cv::line(input,bb.at(0)+cv::Point2f(bb.at(1).x,0),bb.at(0),cv::Scalar(255,255,255),3);

				// draw landmarks
				cv::line(input,lm.at(0),lm.at(1),cv::Scalar(255,255,255),3);
				cv::line(input,lm.at(0),lm.at(2),cv::Scalar(255,255,255),3);
				cv::line(input,lm.at(1),lm.at(2),cv::Scalar(255,255,255),3);
				cv::line(input,lm.at(2),lm.at(3),cv::Scalar(255,255,255),3);
				cv::circle(input,lm.at(0),7,cv::Scalar(255,0,0),-1);
				cv::circle(input,lm.at(1),7,cv::Scalar(255,0,255),-1);
				cv::circle(input,lm.at(2),7,cv::Scalar(0,0,255),-1);
				cv::circle(input,lm.at(3),7,cv::Scalar(0,255,255),-1);

				// show input and output image
				cv::imshow(iwname,input);
				cv::imshow(owname,output);

				char key=cv::waitKey(500);

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
	printf("\n");

	return 0;
}
