#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include "common.h"


/**
algorithm
1. Automatic Canny edge detection and straight line fitting to obtain the set of straight edge
segments;
2. Repeat
(a) Randomly select two segments s1; s2 2 E and intersect them to give the point p;
(b) The support set Sp is the set of straight edges in E going through the point p;
3. Set the dominant vanishing point as the point p with the largest support Sp;
4. Remove all edges in Sp from E and goto 2 for the computation of the next vanishing po
*/
class RansacForVanishPoint
{
public:
	RansacForVanishPoint(std::string& win_name, cv::Mat& src, int edgeThresh = 100);
	~RansacForVanishPoint();
	void CalcVanishingPoints(cv::Point2d vanishing_point_[], int size);
	void Help();
	void DestroyWindow();
private:
	void Init();
	void StartCalc();
	void Reset();
	void Canny(cv::Mat& img_gray, std::vector<Line>& vertical_contours, std::vector<Line>& horizontal_contours);
	void GetLine(cv::Vec4i line, Line& _return);


	/**使用RANSAC获得最佳消失点
	入参： contours  直线段
	出参： vanishing_point  消失点  
	       support_p  最优支撑集，获得当前最优消失点的直线段标记
	*/
	void GetBestVanishingPoint(std::vector<Line>& contours,
		cv::Point2d& vanishing_point,
		short* support_p,
		bool print_info);

	static void Onchange(int, void* object){
		RansacForVanishPoint* ptr = (RansacForVanishPoint*)object;
		ptr->StartCalc();
	}
	static void delete_mouse_callback_static(int event, int x, int y, int flags, void* object){
		RansacForVanishPoint* ptr = (RansacForVanishPoint*)object;
		ptr->delete_mouse_callback(event, x, y,flags);
	}
	void delete_mouse_callback(int event, int x, int y, int flags);
	void GetEraseImg(const cv::Mat& origin, const cv::Mat& mask, cv::Mat _return);
	void ShowCurrentChoose();

	const std::string* win_name_;
	const cv::Mat* src_;
	cv::Mat src_gray_;
	int canny_thresh_ = 100;
	int hough_thresh_ = 70;
	int min_line_length_ = 1;
	int max_line_gap_ = 3;
	int window_width;

	cv::Mat mask_;  //初始化为1，要删去的值修改为0
	uchar rectState_;
	cv::Rect rect_;
	cv::Mat current_canny_edge_;

	cv::Point2d* vanishing_point_;
	int size_;
};

