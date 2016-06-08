#include "common.h"
#include <iostream>
//#include <opencv2/core/core.hpp>
#include <math.h>


void CalibratePoint(cv::Point2d& in_point, const cv::Mat* pseudo_inverse_matrix, cv::Point3d& out_point){
	cv::Mat mat_2d = cv::Mat::ones(cv::Size(1, 3), CV_64F);
	mat_2d.at<double>(0, 0) = in_point.x;
	mat_2d.at<double>(1, 0) = in_point.y;

	cv::Mat mat_3d = *pseudo_inverse_matrix * mat_2d;

	double last = mat_3d.at<double>(3, 0);
	//double last = 1;
	out_point.x = mat_3d.at<double>(0, 0) / last;
	out_point.y = mat_3d.at<double>(1, 0) / last;
	out_point.z = mat_3d.at<double>(2, 0) / last;
}

double EculidDistance_2d(cv::Point2d p1, cv::Point2d p2){
	long temp = (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y);
	return sqrt(temp);
}

double EculidDistance_3d(cv::Point3d p1, cv::Point3d p2){
	long temp = (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y) + (p1.z - p2.z)*(p1.z - p2.z);
	return sqrt(temp);
}

//计算两直线交点
bool CalcIntersection(
	cv::Point2d& _return, cv::Point2d line1_p1, cv::Point2d line1_p2, cv::Point2d line2_p1, cv::Point2d line2_p2){
	double k1 = 0, k2 = 0, b1 = 0, b2 = 0;
	cv::Point2d v1, v2;
	v1.x = line1_p1.x - line1_p2.x;
	v1.y = line1_p1.y - line1_p2.y;
	v2.x = line2_p1.x - line2_p2.x;
	v2.y = line2_p1.y - line2_p2.y;

	bool line1_vertical = false;
	bool line2_vertical = false;
	if (v1.x != 0) {
		k1 = (double)v1.y / v1.x;
		b1 = line1_p1.y - k1*line1_p1.x;
	}
	else{
		//x = m situation
		line1_vertical = true;
		b1 = line1_p1.x;
	}

	if (v2.x != 0) {
		k2 = (double)v2.y / v2.x;
		b2 = line2_p1.y - k2*line2_p1.x;
	}
	else{
		// x = m situation
		line2_vertical = true;
		b2 = line2_p1.x;
	}

	if (!line1_vertical&& !line2_vertical)
	{
		//平行
		if (v1.x*v2.y == v2.x*v1.y){
			return false;
		}
		_return.x = (b1 - b2) / (k2 - k1);
		_return.y = k1*_return.x + b1;
		return true;
	}
	//竖直平行
	if (line1_vertical && line2_vertical){
		return false;
	}

	if (line1_vertical){
		_return.x = b1;
		_return.y = k2*b1 + b2;
		return true;
	}

	if (line2_vertical){
		_return.x = b2;
		_return.y = k1*b2 + b1;
		return true;
	}
	return false;
}

bool GoThroughPoint(cv::Point2d p, Line& line) {
	double d = 0.0;
	if (0 == line.c) {
		if (line.a != 0){
			//y=ax+b相当于ax-y+b=0
			d = abs(line.a * p.x - p.y + line.b);
			double denominator = sqrt(line.a*line.a + 1);
			d = d / denominator;
		}
		else {
			d = abs(line.b - p.y);
		}
	}
	else {
		d = abs(line.c - p.x);
	}
//	std::cout << "point:" << p << "  line:a=" << line.a << " b=" << line.b << " c=" << line.c <<"  d=" << d<<std::endl;
//    std::cout<<"poinnt1 = " << line.point1 << " point2 = " << line.point2 <<std::endl;
	if (d < 2.0) {
		return true;
	} else {
		return false;
	}
}

void ToString(Line& line){
	std::cout << "Line:  point 1:" << line.point1 << "  point 2:" << line.point2 << std::endl;
	std::cout << "a=" << line.a << " b=" << line.b << "c=" << line.c << std::endl;

}

int PowOf2(int value){
	int count = 0;
	unsigned int n = value;
	while (1) {
		if (n >>= 1) count++;
		else break;
	}
	return count;
}

double AngleBetweenTwoLine(Line& line1, Line& line2){
//	ToString(line1); ToString(line2);
	if (line1.c == 0 && line2.c == 0){
		double tan = abs((line1.a - line2.a) / (1 + line1.a*line2.a));
//		std::cout << "tan:" << tan << std::endl;
		return atan(tan)*180/CV_PI;
	}
	if (line1.c != 0 && line2.c != 0){
		return double(0);
	}
	if (line1.c == 0){
		//斜率 a = tan
		double angle = atan(line1.a) * 180 / CV_PI;
		return (angle > 90.0 ? angle - 90.0 : 90.0 - angle);

	}
	if (line2.c == 0){
		double angle = atan(line2.a) * 180 / CV_PI;
		return (angle > 90.0 ? angle - 90.0 : 90.0 - angle);
	}
}