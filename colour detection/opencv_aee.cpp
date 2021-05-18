///////////////////////////////////////////////////////////////
//                                                           //
// H61AEE: Dept. EEE, The University of Nottingham 2018      //
// Author: D. Fallows                                        //
//                                                           //
// opencv_aee.cpp: Library file to simplify OpenCV functions //
//                 Session 6/7 - Computer Vision             //
//                                                           //
///////////////////////////////////////////////////////////////

#include "opencv_aee.hpp"

// Global Variables
VideoCapture cap(0);	// Variable to store the camera device
int cameraFlag = 0;

// Camera functions
void setupCamera(int width, int height)
{
	// Open the camera device for image capture, set the dimensions of the image
	if (!cap.isOpened())
	{
		std::cerr << "ERROR: Unable to open the camera" << std::endl;
		errorTrap();
	}
	cap.set(CAP_PROP_FRAME_WIDTH, width);
	cap.set(CAP_PROP_FRAME_HEIGHT, height);
	cameraFlag = 1;
}

void resizeCamera(int width, int height)
{
    cap.set(CAP_PROP_FRAME_WIDTH, width);
	cap.set(CAP_PROP_FRAME_HEIGHT, height);
}

Mat captureFrame(void)
{
    Mat frame;

    //for(int i = 0; i < 5; i++)
        cap >> frame;

	if(!frame.data)
	{
		std::cout<< "ERROR: Unable to read camera image" << std::endl;
		errorTrap();
	}

	return frame;
}


Point templateMatch(Mat frame, Mat templ, int method, double threshold)
{
    Mat result;
    result.create(frame.rows - templ.rows + 1, frame.cols - templ.cols + 1, CV_32FC1);

    double minVal, maxVal;
    Point minLoc, maxLoc, matchLoc;

    matchTemplate(frame, templ, result, method);

    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
    printf("mat:%f ", maxVal);
	//printf("%f - ", minVal);

    if((maxVal >= threshold) || ((minVal <= threshold) && (method == TM_SQDIFF || method == TM_SQDIFF_NORMED)))
    {
        normalize(result, result, 0, 1, NORM_MINMAX, -1);
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

        if(method  == TM_SQDIFF || method == TM_SQDIFF_NORMED)
            matchLoc = minLoc;
        else
            matchLoc = maxLoc;

        matchLoc.x += templ.cols/2;
        matchLoc.y += templ.rows/2;
    }
    else
    {
		// No valid match found
        matchLoc.x = -1;
        matchLoc.y = -1;
    }

    return matchLoc;
}

Point findContourCentre(std::vector<cv::Point> contour)
{
    Moments foundRegion;    // Variables to store the region moment and the centre point
    Point centre;
    foundRegion = moments(contour, false);      // Calculate the moment for the contour
    centre.x = (foundRegion.m10/foundRegion.m00);  //Calculate the X and Y positions
    centre.y = (foundRegion.m01/foundRegion.m00);

    return centre;
}


Mat readImage(const char* imageLoc)
{
	Mat image = imread(imageLoc, IMREAD_COLOR);

	if(!image.data)
	{
		std::cout<< "ERROR: Unable to read image data" << std::endl;
		errorTrap();
	}

	return image;
}

void errorTrap(void)
{
    while(1);
}

void closeCV(void)
{
    // Clean up the open camera and windows
    if(cameraFlag)
        cap.release();
	destroyAllWindows();
}

Mat transformPerspective(std::vector<Point> boundingContour, Mat frame, int x_size, int y_size)
{
    if(boundingContour.size() != 4)
    {
        // Error the contour has too many points. Only 4 are allowed
        Mat emptyMat;
        return emptyMat;
    }

    Mat symbol(y_size,x_size,CV_8UC1, Scalar(0));

    Point2f symbolCorners[4], boundingCorners[4];      // Create (and populate) variables containing the corner locations for the transform
    symbolCorners[0] = Point2f(0,0);
    symbolCorners[1] = Point2f(symbol.cols - 1,0);
    symbolCorners[2] = Point2f(symbol.cols - 1,symbol.rows - 1);
    symbolCorners[3] = Point2f(0,symbol.rows - 1);

    Point contourCentre = findContourCentre(boundingContour);   // To populate the contour corners we need to check the order of the points

    int point1, point2, point3, point4;

    if(boundingContour[0].x > contourCentre.x)
    {
        if(boundingContour[0].y > contourCentre.y)
            point3 = 0;
        else
            point2 = 0;
    }
    else
    {
        if(boundingContour[0].y > contourCentre.y)
            point4 = 0;
        else
            point1 = 0;
    }

    if(boundingContour[1].x > contourCentre.x)
    {
        if(boundingContour[1].y > contourCentre.y)
            point3 = 1;
        else
            point2 = 1;
    }
    else
    {
        if(boundingContour[1].y > contourCentre.y)
            point4 = 1;
        else
            point1 = 1;
    }

    if(boundingContour[2].x > contourCentre.x)
    {
        if(boundingContour[2].y > contourCentre.y)
            point3 = 2;
        else
            point2 = 2;
    }
    else
    {
        if(boundingContour[2].y > contourCentre.y)
            point4 = 2;
        else
            point1 = 2;
    }

    if(boundingContour[3].x > contourCentre.x)
    {
        if(boundingContour[3].y > contourCentre.y)
            point3 = 3;
        else
            point2 = 3;
    }
    else
    {
        if(boundingContour[3].y > contourCentre.y)
            point4 = 3;
        else
            point1 = 3;
    }

    if(point1 + point2 + point3 + point4 != 6)
    {
        //Unable to reconstruct rectangle
        Mat emptyMat;
        return emptyMat;
    }

    boundingCorners[0] = boundingContour[point1];
    boundingCorners[1] = boundingContour[point2];
    boundingCorners[2] = boundingContour[point3];
    boundingCorners[3] = boundingContour[point4];

    Mat transformMatrix = cv::getPerspectiveTransform(boundingCorners, symbolCorners); // Calculate the required transform operation
    Mat transformedSymbol(240,320,CV_8UC1,Scalar(0));
    warpPerspective(frame, transformedSymbol, transformMatrix, cv::Size(symbol.cols,symbol.rows));  // Perform the transformation

    return transformedSymbol;
}

float compareImages(Mat cameraImage, Mat librarySymbol)
{
    float matchPercent = 100 - (100/((float)librarySymbol.cols*(float)librarySymbol.rows) * (2*(float)countNonZero(librarySymbol^cameraImage))); // An incorrect pixel has double weighting
    return matchPercent;
}
