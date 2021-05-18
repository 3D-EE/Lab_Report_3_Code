// Include files for required libraries
#include <stdio.h>

#include "opencv_aee.hpp"
#include "main.hpp" // You can use this file for declaring defined values and functions
#include <iostream>
#include "wiringPi.h"

#include "pi2c.h"

using namespace std;

void setup(void)
{
    setupCamera(320, 240); // Enable the camera for OpenCV
}

int main(int argc, char **argv)
{
    setup(); // Call a setup function to prepare IO and devices

    Pi2c arduino(9);    //set up zi2c with addresses
    Pi2c MPU(68);



    cv::namedWindow("Photo"); // Create a GUI window called photo

    Mat  DistanceMeasurement, Football, GreenShortCut, RedShortCut, ShapeCounter, StopLight;

    Mat  DistanceMeasurement_HSV, Football_HSV, GreenShortCut_HSV, RedShortCut_HSV, ShapeCounter_HSV, StopLight_HSV;

    Mat  DistanceMeasurement_GREY, Football_GREY, GreenShortCut_GREY, RedShortCut_GREY, ShapeCounter_GREY, StopLight_GREY;


    DistanceMeasurement = readImage("DistanceMeasurement.PNG");
    Football = readImage("Football.PNG");
    GreenShortCut = readImage("GreenShortCut.PNG");
    RedShortCut = readImage("RedShortCut.PNG");
    ShapeCounter = readImage("ShapeCounter.PNG");
    StopLight = readImage("StopLight.PNG");

    cvtColor(DistanceMeasurement, DistanceMeasurement_HSV, COLOR_BGR2HSV);
    cvtColor(Football, Football_HSV, COLOR_BGR2HSV);
    cvtColor(GreenShortCut, GreenShortCut_HSV, COLOR_BGR2HSV);
    cvtColor(RedShortCut, RedShortCut_HSV, COLOR_BGR2HSV);
    cvtColor(ShapeCounter, ShapeCounter_HSV, COLOR_BGR2HSV);
    cvtColor(StopLight, StopLight_HSV, COLOR_BGR2HSV);

    inRange(DistanceMeasurement_HSV, Scalar(100, 40, 70), Scalar(179, 255, 255), DistanceMeasurement_GREY); //pink
    inRange(Football_HSV, Scalar(100, 40, 70), Scalar(179, 255, 255), Football_GREY);                       //pink
    inRange(GreenShortCut_HSV, Scalar(100, 40, 70), Scalar(179, 255, 255), GreenShortCut_GREY);             //pink
    inRange(RedShortCut_HSV, Scalar(100, 40, 70), Scalar(179, 255, 255), RedShortCut_GREY);                 //pink
    inRange(ShapeCounter_HSV, Scalar(100, 40, 70), Scalar(179, 255, 255), ShapeCounter_GREY);               //pink
    inRange(StopLight_HSV, Scalar(100, 40, 70), Scalar(179, 255, 255), StopLight_GREY);                     //pink

    Mat image_array[7];

    //zero reserved for later max determination
    image_array[1] = DistanceMeasurement_GREY;
    image_array[2] = Football_GREY;
    image_array[3] = GreenShortCut_GREY;
    image_array[4] = RedShortCut_GREY;
    image_array[5] = ShapeCounter_GREY;
    image_array[6] = StopLight_GREY;


    while (1) // Main loop to perform image processing
    {
        Mat frame, flip_frame;
        Mat image_HSV, image_GREY_pink, image_GREY_black;
        Mat transformed, transformed_HSV, transformed_GREY;
        //Mat grayscale, equalised;

        while (frame.empty())
            frame = captureFrame(); // Capture a frame from the camera and store in a new matrix variable

        cv::flip(frame, flip_frame, -1); //rotate image

        cvtColor(flip_frame, image_HSV, COLOR_BGR2HSV);                                  // Convert the image to HSV
        inRange(image_HSV, Scalar(100, 40, 70), Scalar(179, 255, 255), image_GREY_pink); //look for pink

        //cvtColor(flip_frame, grayscale, COLOR_BGR2GRAY);
        //equalizeHist(grayscale, equalised); // Improve the contrast

        int numNonZero = countNonZero(image_GREY_pink);

        if(numNonZero < 1000)   //isnt much pink, so assume line following
        {
            cout << "Very little pink\n" << endl;
            inRange(image_HSV, Scalar(0, 0, 0), Scalar(179, 255, 40), image_GREY_black); //look for black line
            //cv::imshow("B&W", image_GREY_black); //Display
            //cv::waitKey(1);

            std::vector<std::vector<cv::Point>> contours_black;                                                        // Variable for list of contours
            std::vector<Vec4i> hierarchy_black;                                                                        // Variable for image topology data
            cv::findContours(image_GREY_black, contours_black, hierarchy_black, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0)); // Calculate the contours and store them

            int num_contours_black = contours_black.size();
            //cout << "\nNum of contours: " << num_contours_black << "\n" << endl;

            int area_black;
            int maxArea_black;

            if(num_contours_black >= 1)
            {

                cout << "Contours detected, line following\n" << endl;
                for (int i = 0; i < num_contours_black; i++) // Loop through the contours
                {
                    drawContours(flip_frame, contours_black, i, Scalar(0,0,255), 2, LINE_8, noArray(), 0, Point() ); // Draw each in red
                    area_black = cv::contourArea(contours_black[i]);
                    //cout << i << " " << area_black << endl;

                    if (i == 0)
                        maxArea_black = area_black; //initialise max area

                    if (area_black > maxArea_black)
                    {
                        contours_black[0] = contours_black[i];
                        maxArea_black = area_black;
                        cout << maxArea_black << "\n"<< endl;
                    }
                }

                if(maxArea_black > 4000) //check if largest areais significant
                {
                    std::vector<cv::Point> approxedcontours_black;
                    cv::approxPolyDP(contours_black[0], approxedcontours_black, 10, true);



                    Point regionCentre = findContourCentre(approxedcontours_black); // Calculate the centre point
                    //printf("Contour centre: x = %dpx, y = %dpx\n", regionCentre.x, regionCentre.y);

                    int mapped_regionCentre = (regionCentre.x) * (75-25) / 320 + 25;

                    //format of map (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
                    cout << "(50 is centre)\nPosition: " << mapped_regionCentre << "\n" << endl;

                    cv::circle(flip_frame, Point(160,120), 10, Scalar(60,235, 240),  FILLED); //green dot at centre of image
                    cv::circle(flip_frame, regionCentre, 10, Scalar(0,0,255),  FILLED);   //red dot at centre of contour

                    char lf_char[1] = {'S'}; //line following command (S for follow snakey line)
                    arduino.i2cWrite(lf_char, 1);
                    arduino.i2cWriteArduinoInt(mapped_regionCentre);
                    delay(50);
                }


            }

            if(num_contours_black < 1 || maxArea_black < 1000) //largest contour too small (not valid), turn off motors
            {
                cout << "Invalid contour\n" << endl;
                char p_char[1] ={'P'}; //pause command
                arduino.i2cWrite(p_char, 1);
            }
        }

        std::vector<std::vector<cv::Point>> contours;                                                        // Variable for list of contours
        std::vector<Vec4i> hierarchy;                                                                        // Variable for image topology data
        cv::findContours(image_GREY_pink, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0)); // Calculate the contours and store them

        int num_contours = contours.size();
        cout << "\nNum of contours: " << num_contours << "\n" << endl;

        if (num_contours >= 1)
        {
            int area;
            int maxArea;

            for (int i = 0; i < contours.size(); i++) // Loop through the contours
            {
                //drawContours(flip_frame, contours, i, Scalar(0,0,255), 2, LINE_8, noArray(), 0, Point() ); // Draw each in red
                area = cv::contourArea(contours[i]);
                cout << i << " " << area << endl;


                if (i == 0)
                    maxArea = area; //initialise max area

                if (area > maxArea)
                {
                    contours[0] = contours[i];
                    maxArea = area;
                }
            }

            //cout << "Max area: " << maxArea << "\n" << endl;
            if( maxArea > 5000)  //don't transform if all contours are small
            {
                std::vector<cv::Point> approxedcontours;
                cv::approxPolyDP(contours[0], approxedcontours, 10, true);

                transformed = transformPerspective(approxedcontours, flip_frame, 320, 240);
            }

            if (transformed.empty() == 1)
            {
                cout << "Can't transform\n"  << endl;
            }

            if (transformed.empty() == 0)
            {

                cvtColor(transformed, transformed_HSV, COLOR_BGR2HSV);                                  // Convert the image to HSV so we can compare to symbol
                inRange(transformed_HSV, Scalar(100, 40, 70), Scalar(179, 255, 255), transformed_GREY); //look for pink - B&W transformed image

                float match_percentage[6];
                int img_num = 1;

                float best_match = match_percentage[0];
                int best_img_num = 0;

                for (img_num; img_num <= 6; img_num++) //change to size of iamges loaded in ///fix
                {
                    match_percentage[img_num] = compareImages(transformed_GREY, image_array[img_num]);
                    //cout << "\nImage no: " << img_num << "\tMatch percentage: " << match_percentage[img_num] << "\n\n"     << endl;
                    if (match_percentage[img_num] > best_match)
                    {
                        best_match = match_percentage[img_num];
                        best_img_num = img_num;
                    }
                }

                if(best_match > 30) //don't bother displaying message if match is terrible
                {
                    cout << "\n\n Best match is image: " << best_img_num << "\tSimilarity: " << best_match << "\n"  << endl;


                    switch(best_img_num)
                    {
                    case 3:
                    {
                        char l_char[1] = {'L'};
                        arduino.i2cWrite(l_char,1);
                        break;
                    }
                    case 4:
                    {
                        char r_char[1] = {'R'};
                        arduino.i2cWrite(r_char, 1);
                        break;
                    }

                    case 6:
                    {
                        char t_char[1] = {'T'};
                        arduino.i2cWrite(t_char, 1);
                        break;
                    }
                    case 2:
                    {
                        char o_char[1] = {'O'};
                        arduino.i2cWrite(o_char, 1);
                        break;
                    }
                    default:
                        cout << "Shape counting\n" <<endl;
                    }




                    if (best_img_num == 1 || best_img_num == 5) //1 - distancemeasurment, 5 - shape counter ///fix
                    {
                        std::vector<std::vector<cv::Point>> new_contours;                                                             // Variable for list of contours
                        std::vector<Vec4i> new_hierarchy;                                                                             // Variable for image topology data
                        cv::findContours(transformed_GREY, new_contours, new_hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0)); // Calculate the contours and store them

                        /*
                        std::vector< std::vector <cv::Point>> new_approxedcontours(new_contours.size());

                        for(int p = 0; p < new_contours.size(); p++)
                        {
                            cv::approxPolyDP(Mat(new_contours[p]), new_approxedcontours[p], 2, true);

                        }

                        for (int z = 0; z < new_approxedcontours.size(); z++) // Loop through the contours
                        {

                            drawContours(transformed, new_approxedcontours, z, Scalar(0, 0, 255), 2, LINE_8, noArray(), 0, Point()); // Draw each in red
                        }

                        int new_num_contours = new_approxedcontours.size();
                        */


                        /*
                        for (int z = 0; z < new_contours.size(); z++) // Loop through the contours
                        {
                            //drawContours(transformed, new_contours, z, Scalar(0, 0, 255), 2, LINE_8, noArray(), 0, Point()); // Draw each in red
                            int new_area = cv::contourArea(new_contours[z]);
                            cout << z << " Area: " << new_area  << "\n" << endl;

                            if (new_area < 500)
                            {
                                new_contours.erase(new_contours.begin() + z);

                            }

                        }

                        */

                        int new_num_contours = new_contours.size();


                        cout << "\nTransformed - Num of contours: " << new_num_contours << "\n" << endl;

                        if(best_img_num == 5)
                        {

                            char f_char[1] = {'F'};
                            arduino.i2cWrite(f_char, 1);
                            arduino.i2cWriteArduinoInt(new_num_contours);
                            delay(500);
                        }

                        if(best_img_num == 1)
                        {
                            char b_char[1] = {'B'};
                            arduino.i2cWrite(b_char, 1);
                            arduino.i2cWriteArduinoInt(new_num_contours);
                            delay(500);
                        }

                    }

                    //cv::imshow("Transformed", transformed);
                    cv::imshow("Transformed_grey", transformed_GREY);
                }
            }
        }

        cv::imshow("Photo", flip_frame); //Display the flipped image in the window
        //cv::imshow("Test", equalised);
        //cv::imshow("B&W", image_GREY_pink);

        int key = cv::waitKey(1); // Wait 1ms for a keypress (required to update windows)

        key = (key == 255) ? -1 : key; // Check if the ESC key has been pressed
        if (key == 27)
        {
            char p_char[1] = {'P'}; //pause comman
            arduino.i2cWrite(p_char, 1);
            break;
        }

    }

    closeCV(); // Disable the camera and close any windows

    return 0;

}
