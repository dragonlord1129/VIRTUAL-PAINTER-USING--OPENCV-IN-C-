#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class VirtualPainter {
private:
    Mat imgHSV;
    vector<vector<int>> myColors;
    vector<Scalar> myColorValues;

public:
    VirtualPainter() {
        // Initialize myColors and myColorValues
         myColors = {{108, 31, 29, 141, 255, 237},  // Blue
                     {22, 0, 0, 114, 255, 56},      // Green
                     {124, 174, 69, 179, 255, 197}  // Red
                   };
        myColorValues = { Scalar(255, 0, 0),  // Blue
                          Scalar(0, 255, 0),  // Green
                          Scalar(0, 0, 255)   // Red
                        };
    }

    vector<vector<int>> findColors(Mat img);
    vector<Scalar> getmyColorValues();
    static Point getContours(Mat image, VirtualPainter& p);
};

vector<vector<int>> VirtualPainter::findColors(Mat img) {
    vector<vector<int>> newPoints;
    cvtColor(img, imgHSV, COLOR_BGR2HSV);
    for (size_t i = 0; i < myColors.size(); i++) {
        Scalar lower(myColors[i][0], myColors[i][1], myColors[i][2]);
        Scalar upper(myColors[i][3], myColors[i][4], myColors[i][5]);
        Mat mask;
        inRange(imgHSV, lower, upper, mask);
        Point stuffPoint = getContours(mask, *this);
        if (stuffPoint.x != 0) {
            newPoints.push_back({ stuffPoint.x, stuffPoint.y, int(i) });
        }
    }
    return newPoints;
}

vector<Scalar> VirtualPainter::getmyColorValues() {
    return myColorValues;
}

Point VirtualPainter::getContours(Mat img, VirtualPainter& p) {
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    vector<vector<Point>> conPoly(contours.size());
    vector<Rect> boundRect(contours.size());
    Point myPoint(0, 0);
    for (size_t i = 0; i < contours.size(); i++) {
        int area = contourArea(contours[i]);
        if (area > 1000) {
            float peri = arcLength(contours[i], true);
            approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
            boundRect[i] = boundingRect(conPoly[i]);
            myPoint.x = boundRect[i].x + boundRect[i].width / 2;
            myPoint.y = boundRect[i].y;
        }
    }
    return myPoint;
}

void draw(Mat& canvas, vector<vector<int>> newPoints, vector<vector<int>>& prevPoints, vector<Scalar> myColorValues) {
    for (size_t i = 0; i < newPoints.size(); i++) {
        if (!prevPoints[i].empty()) {
            // Draw a line between the current and previous points on the canvas
            line(canvas, Point(prevPoints[i][0], prevPoints[i][1]), Point(newPoints[i][0], newPoints[i][1]), myColorValues[newPoints[i][2]], 10);
        }
        // Update the previous points to the current points for the next iteration
        prevPoints[i] = newPoints[i];
    }
}

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Error: Unable to open camera" << endl;
        return -1;
    }

    VirtualPainter obj;
    vector<vector<int>> prevPoints(obj.getmyColorValues().size());

    Mat canvas;

    while (true) {
        Mat img;
        cap.read(img);
        if (img.empty()) {
            cout << "Error: Unable to read frame" << endl;
            break;
        }

        if(canvas.empty()){
            //Initialize the canvas with same size as the input image
            canvas = Mat(img.size(), CV_8UC3, Scalar(255, 255, 255));
        }
        // Draw on the canvas
        vector<vector<int>> newPoints = obj.findColors(img);
        draw(canvas, newPoints, prevPoints, obj.getmyColorValues());
        
        Mat blendedImg;
        addWeighted(img, 1, canvas, 0.5, 0, blendedImg);

        imshow("Press ESC to end the drawing", blendedImg);
        char c = waitKey(1);
        if (c == 27) // Esc key to exit
            break;
        else if (c == 'c' || c == 'C') { // Clear canvas if 'c' or 'C' is pressed
            canvas = Mat(img.size(), CV_8UC3, Scalar(255, 255, 255));
            prevPoints.clear();
            prevPoints.resize(obj.getmyColorValues().size());
        }
    }

    return 0;
}
