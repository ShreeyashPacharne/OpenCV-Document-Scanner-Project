#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <conio.h>

using namespace std;
using namespace cv;

///		Project 2 Document Scanner 		///


Mat img, imgGrayscale, imgCannyEdge, imgGaussblur, imgdil, imgTreshold, matrix, imgwarp, img_final;
vector<Point> initialpoints, docpoints;
float w = 420, h = 596;

//	PREPROCESSING
Mat preprocessing(Mat img_input)
{
	
		cvtColor(img_input, imgGrayscale, COLOR_BGR2GRAY);
		GaussianBlur(imgGrayscale, imgGaussblur, Size(3, 3), 3, 3);
		Canny(imgGaussblur, imgCannyEdge, 122, 231);
		//Canny(imgGaussblur, imgCannyEdge, 527, 184);
		Mat kernel = getStructuringElement(MORPH_RECT, Size(2, 2));
		dilate(imgCannyEdge, imgdil, kernel);

		return imgdil;
	
}

// GET CONTOURS THE BIGGEST RECTANGLE

vector<Point> getcontours(Mat image)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> conpoly(contours.size());

	vector<Point> bigger;
	int maxArea=0;

	for (int i = 0; i < contours.size(); i++)		//this is used to not detect the noise in the image
	{
		int area = contourArea(contours[i]);

		if (area >= 1000)
		{

			float peri = arcLength(contours[i], true);		//for counting the number of curves or points
			approxPolyDP(contours[i], conpoly[i], 0.02 * peri, true);

			if (area > maxArea && conpoly[i].size() == 4)
			{
				//drawContours(img, conpoly, i, Scalar(0, 0, 255), 3);
				bigger = { conpoly[i][0] , conpoly[i][1], conpoly[i][2], conpoly[i][3] };
				maxArea = area;
			}

			//drawContours(img, conpoly, i, Scalar(0, 0, 255), 3);	

		}
	}
	return bigger;
}

void drawpoints(vector<Point> points, Scalar Color)
{
	for (int i = 0; i < points.size(); i++)
	{
		circle(img, points[i], 5, Color, FILLED);
		putText(img, to_string(i), points[i], FONT_HERSHEY_DUPLEX, 1, Color, 3);
	}
}

vector<Point> reorder(vector<Point> points)
{
	vector<Point> newpoints;
	vector<int> sumPoints, subPoints;

	for (int i = 0; i < 4; i++)
	{
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}
	newpoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);
	newpoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
	newpoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
	newpoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);

	return newpoints;
}


Mat warping(Mat img, vector<Point> point, float w, float h)
{
	Point2f src[4] = { point[0], point[1] ,point[2], point[3] };
	Point2f dist[4] = { { 0.0f,0.0f }, { w,0.0f }, { 0.0f,h }, { w,h } };

	matrix = getPerspectiveTransform(src, dist);
	warpPerspective(img, imgwarp, matrix, Point(w, h));
	return imgwarp;
}


void main()
{
	string path = "resources/three.jpg";
	img = imread(path);
	
	resize(img, img, Size(), 0.5, 0.5);
	
	
	//Preprocessing
	imgTreshold = preprocessing(img);

	////GetContours
	initialpoints = getcontours(imgTreshold);
	//drawpoints(initialpoints, Scalar(0, 0, 255));
	docpoints = reorder(initialpoints);


	////Warp
	img_final = warping(img, docpoints, w, h);
	drawpoints(docpoints, Scalar(255, 0, 255));

	////crop
	int cropVal = 10;
	Rect roi(cropVal, cropVal, w - (2 * cropVal), h - (2 * cropVal));
	Mat imgcrop = img_final(roi);


	imshow("Image Original", img);
	imshow("Image marked", imgdil);
	imshow("Image final", imgwarp);
	imshow("Image cropped", imgcrop);
	waitKey(0);
}