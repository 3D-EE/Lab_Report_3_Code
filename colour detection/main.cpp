// Include files for required libraries
#include <stdio.h>

#include "opencv_aee.hpp"
#include "main.hpp"     // You can use this file for declaring defined values and functions

using namespace std;

void setup(void)
{
    /// Setup camera won't work if you don't have a compatible webcam
    //setupCamera(320, 240);  // Enable the camera for OpenCV
}

int main( int argc, char** argv )
{
    setup();    // Call a setup function to prepare IO and devices

    while(1)    // Main loop to perform image processing
    {
        Mat frame;
        Mat image_HSV;
        //Mat image_GREY;
        Mat image_GREY_blue;
        Mat image_GREY_green;
        Mat image_GREY_red;

        // Can't capture frames without a camera attached. Use static images instead
        while(frame.empty())
        {
            /// Can't capture frames without a camera attached. Use static images instead
            //frame = captureFrame(); // Capture a frame from the camera and store in a new matrix variable

            frame = readImage("BlueApple.bmp");
            //frame = readImage("BlueCar.bmp");
            //frame = readImage("GreenApple.bmp");
            //frame = readImage("GreenCar.bmp");
            //frame = readImage("RedApple.bmp");
            //frame = readImage("RedCar.bmp");



        }



        cvtColor(frame, image_HSV, COLOR_BGR2HSV); // Convert the image to HSV

        //inRange(image_HSV, Scalar(25, 20, 70), Scalar(140, 255, 240), image_GREY); ///blue with shadow, better apple
        //inRange(image_HSV, Scalar(90,100,50), Scalar(110,255,255), image_GREY); ///good blue apple
        //inRange(image_HSV, Scalar(120,120,120), Scalar(190,255,255), image_GREY);  //pink

        inRange(image_HSV, Scalar(100, 25, 0), Scalar(140, 255, 250), image_GREY_blue); //checking for blue
        inRange(image_HSV, Scalar(38, 50, 0), Scalar(75, 230, 250), image_GREY_green); //checking for green
        inRange(image_HSV, Scalar(0, 0, 0), Scalar(10, 255, 250), image_GREY_red); //checking for red
//37.5, 97, 64
        //int numNonZero = 0; // Create a integer to store the result
        //numNonZero = countNonZero(image_GREY); // Count the number of non-zero pixels

        /*
        cout << "Number of blue pixels is: " << numNonZero << "\n";
        float percentageBlue;
        percentageBlue = (((float)numNonZero / (320*240)) * 100);
        cout << "Percentage blue: " << percentageBlue << "\n";
        */

        cv::imshow("Photo", frame); //Display the image in the window
        cv::imshow("PhotoB", image_GREY_blue);
        cv::imshow("PhotoG", image_GREY_green);
        cv::imshow("PhotoR", image_GREY_red);

        int bluePixels = 0, greenPixels = 0, redPixels = 0, totalPixels = 320*240;

        bluePixels = countNonZero(image_GREY_blue);
        greenPixels = countNonZero(image_GREY_green);
        redPixels = countNonZero(image_GREY_red);


        if(bluePixels >= greenPixels && bluePixels >= redPixels) //check if image primarily blue
        {
            cout << "This image is blue" << "\n\n";
        }

        else if(greenPixels >= bluePixels && greenPixels >= redPixels) //check if image primarily green
        {
            cout << "This image is green" << "\n\n";
        }

        else if(redPixels >= bluePixels && redPixels >= greenPixels) //check if image primarily red
        {
            cout << "This image is red" << "\n\n";
        }

        else
            cout << "Could not determine most significant colour\n";


        int key = cv::waitKey(1);   // Wait 1ms for a keypress (required to update windows)

        key = (key==255) ? -1 : key;    // Check if the ESC key has been pressed
        if (key == 27)
            break;
	}

	closeCV();  // Disable the camera and close any windows

	return 0;
}



