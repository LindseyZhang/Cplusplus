//#include <stdlib.h>
#include "RansacForVanishPoint.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <Windows.h>
#include <ctime>
#include "common.h"

using namespace cv;
using namespace std;

static double min_vertical_k = 5.67;     // angle = 80

RansacForVanishPoint::RansacForVanishPoint(std::string& win_name, cv::Mat& src, int canny_thresh)
{
	if (win_name.empty()) {
		std::cout << "RansacForVanishPoint: win_name is empty " << std::endl;
		return;
	}
	if (src.empty()) {
		std::cout << "RansacForVanishPoint:  src is empty" << std::endl;
		return;
	}

	win_name_ = &win_name;
	src_ = &src;
	canny_thresh_ = canny_thresh;

	src_gray_.create(src_->size(), src_->type());
	cv::cvtColor(*src_, src_gray_, CV_BGR2GRAY);
	cv::imshow(*win_name_, *src_);

	window_width = src_->size().width;
	int screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
	window_width = window_width < screen_width/3 ? window_width : screen_width/3;

	mask_.create(src_->size(), CV_8UC1);
	Reset();
	Init();
}


RansacForVanishPoint::~RansacForVanishPoint()
{
}


void RansacForVanishPoint::Init() {
	cv::namedWindow(TRACKBAR_WINDOW, cv::WINDOW_NORMAL);
	cv::resizeWindow(TRACKBAR_WINDOW, 800, 200);
	cv::moveWindow(TRACKBAR_WINDOW, 80, 10);

	//Canny 边缘检测结果窗口
	cv::moveWindow(*win_name_, 80, 230);

	//Hough变换提取直线的结果窗口
	cv::namedWindow(ALL_LINES_WINDOW, CV_WINDOW_AUTOSIZE);
	cv::moveWindow(ALL_LINES_WINDOW, 80 + window_width, 230);

	//根据提取的直线计算不同方向消失点的结果窗口
	namedWindow(LINES_USED_TO_CALC_VANISHING_WINDOW);
	cv::moveWindow(LINES_USED_TO_CALC_VANISHING_WINDOW, 80 + 2 * window_width, 230);
}

void RansacForVanishPoint::DestroyWindow() {
	cv::destroyWindow(*win_name_);
	cv::destroyWindow(TRACKBAR_WINDOW);
	cv::destroyWindow(ALL_LINES_WINDOW);
	cv::destroyWindow(LINES_USED_TO_CALC_VANISHING_WINDOW);	
}
void RansacForVanishPoint::Help(){
	cout << "\nChange value of parameter in Trackbar panel\n"
		"Hot key.\n"
	//	"\tctrl - clear erase\n"
		"\tleft mouse button - set rectangle and erase the content inside\n" << std::endl;
}

void RansacForVanishPoint::Reset(){	
	if (!mask_.empty())
		mask_.setTo(Scalar::all(1));
	rectState_ = NOT_SET;
}

void RansacForVanishPoint::CalcVanishingPoints(cv::Point2d vanishing_point[], int size){
	if (size < 3){
		cout << "消失点数组的大小必须大于3" << endl;
		return;
	}
	vanishing_point_ = vanishing_point;
	size_ = size;
	
	StartCalc();

	cv::createTrackbar("Canny Threshold", TRACKBAR_WINDOW,
		&canny_thresh_, 255, &RansacForVanishPoint::Onchange, this);
	cv::createTrackbar("Hough Threshold", TRACKBAR_WINDOW,
		&hough_thresh_, 255, &RansacForVanishPoint::Onchange, this);
	cv::createTrackbar("MinLineLength", TRACKBAR_WINDOW,
		&min_line_length_, 100, &RansacForVanishPoint::Onchange, this);
	cv::createTrackbar("MaxLineGap", TRACKBAR_WINDOW, 
		&max_line_gap_, 100, &RansacForVanishPoint::Onchange, this);
}

void RansacForVanishPoint::StartCalc() {
	cv::Mat process_img;
//	GetEraseImg(src_gray_, mask_, process_img);

	std::vector<Line> vertical_contours,horizontal_contours;
	Canny(src_gray_, vertical_contours,horizontal_contours);

	cv::Mat used_line;
	src_->copyTo(used_line);

	int best = 0;
	int numDataObjects = vertical_contours.size();
	short *bestVotes = new short[numDataObjects];  //one if data[i] agrees with the best model, otherwise zero  
	for (int i = 0; i < numDataObjects; ++i) {
		bestVotes[i] = 0;
	}


	std::cout << "\n\ncalc first vanishing point..." << std::endl;
	GetBestVanishingPoint(vertical_contours, vanishing_point_[0], bestVotes,false);
	std::cout << "first vanishing point:" << vanishing_point_[0] << std::endl;
	
	//将计算该消失点的线段移出集合，计算下一个消失点 
//	std::vector<Line> contours_1;
	for (int j = 0; j < numDataObjects; ++j) {
		if (bestVotes[j] == 1) {
			//line(used_line, contours[j].point1, contours[j].point2, Scalar(0, 0, 255), 1, CV_AA);
			line(used_line, vertical_contours[j].point1, vertical_contours[j].point2, Scalar(0, 0, 255), 2, CV_AA);
		} /*else {
			contours_1.push_back(contours[j]);
		}*/
	}
	
//	contours.swap(contours_1);
//	std::cout << "contours size:" << contours.size() << std::endl;
//	std::cout << "contours_1 size:" << contours_1.size() << std::endl;  */
	//// 对局内点再次用最小二乘法拟合出模型  
	//paramEstimator->leastSquaresEstimate(leastSquaresEstimateData, parameters);

	std::cout << "calc second vanishing point..." << std::endl;
	//计算第二个消失点
	short* wait_delete = bestVotes;
//	numDataObjects = contours.size();
	numDataObjects = horizontal_contours.size();
	bestVotes = new short[numDataObjects];
	for (int i = 0; i < numDataObjects; ++i){
		bestVotes[i] = 0;
	}
	delete[] wait_delete;
	GetBestVanishingPoint(horizontal_contours, vanishing_point_[1], bestVotes,false);
	std::cout << "second vanishing point:" << vanishing_point_[1] << std::endl;

	//剔除计算第二个消失点用到的直线 TODO
//	contours_1.clear();
	std::vector<Line> contours_1;
	for (int j = 0; j < numDataObjects; j++) {
		if (bestVotes[j] == 1){
			//line(used_line, contours[j].point1, contours[j].point2, Scalar(0, 255, 0), 1, CV_AA);
			line(used_line, horizontal_contours[j].point1, horizontal_contours[j].point2, Scalar(0, 255, 0), 2, CV_AA);
		} else {
			//contours_1.push_back(contours[j]);
			contours_1.push_back(horizontal_contours[j]);
		}
	}
	//contours.swap(contours_1);
	horizontal_contours.swap(contours_1);
	std::cout << " contours size:" << horizontal_contours.size() << std::endl;;
	std::cout << "  contours_1 size:" << contours_1.size() << std::endl;

	std::cout << "calc third vanishing point..." << std::endl;
	//计算第三个消失点
	wait_delete = bestVotes;
//	numDataObjects = contours.size();
	numDataObjects = horizontal_contours.size();
	bestVotes = new short[numDataObjects];
	delete[] wait_delete;
	GetBestVanishingPoint(horizontal_contours, vanishing_point_[2], bestVotes,false);
	std::cout << "third vanishing point:" << vanishing_point_[2] << std::endl;

	for (int j = 0; j < numDataObjects; j++) {
		if (bestVotes[j] == 1){
		//	line(used_line, contours[j].point1, contours[j].point2, Scalar(255, 0, 0), 1, CV_AA);
			line(used_line, horizontal_contours[j].point1, horizontal_contours[j].point2, Scalar(255, 0, 0), 2, CV_AA);
		}
	}

	imshow(LINES_USED_TO_CALC_VANISHING_WINDOW, used_line);
	std::cout << "finish calc vanishing point\n" << std::endl;
	std::cout << "Press any key in OpenCV window to finish set vanishing point,"<<std::endl
		<<"then press n/N to continue..." << std::endl;
}

// 计算所有可能的直线，寻找其中误差最小的解。
//对于100点的直线拟合来说，大约需要100*99*0.5=4950次运算 
void RansacForVanishPoint::GetBestVanishingPoint(
	vector<Line>& contours,
	cv::Point2d& vanishing_point,
	short* best_votes,bool print_info){
	int size = contours.size();
	if (size < 2){
		cout << "\nWRONG: contours size is less than 2,can't get vanishing point" << endl;
		return;
	}

	short *cur_votes = new short[size];
	int best_count = 0;
	int cur_count = 0;
	for (int outer = 0; outer < size - 1; ++outer){
		for (int inner = outer+1; inner < size; ++inner){
			
			double angle = AngleBetweenTwoLine(contours[outer], contours[inner]);
			if (angle > 45.0){
				continue;
			}
			cv::Point2d point;
			if (!CalcIntersection(point, contours[outer].point1, contours[outer].point2,
				contours[inner].point1, contours[inner].point2)){
				continue;
			}

			//重置cur_votes
			for (int j = 0; j < size; ++j){
				cur_votes[j] = 0;
			}
			cur_count = 0;

			
			if (print_info){
				ToString(contours[outer]); ToString(contours[inner]);
				std::cout << "intersector:" << point << std::endl;
			}

			for (int j = 0; j < size; ++j){
				if (GoThroughPoint(point, contours[j])){
					if (print_info){
						ToString(contours[j]);
					}
					++cur_count;
					cur_votes[j] = 1;
				}
			}
			if (print_info){
				std::cout << "count:" << cur_count << std::endl << std::endl;
			}
			if (best_count < cur_count){
				best_count = cur_count;
				vanishing_point = point;
				for (int k = 0; k < size; ++k){
					best_votes[k] = cur_votes[k];
				}
			}
			
		}   //end inner for loop
	}   // end outer for loop

/*	for (int i = 0; i < run_time; ++i){
		//重置cur_votes
		for (int j = 0; j < size; ++j){
			cur_votes[j] = 0;
		}
		cur_count = 0;

		time_t seed = time(NULL);
		srand(seed*(i + 1));
		int random_1 = rand() % size;
		int random_2;
		double angle;
		int count = 0;
		do{
			random_2 = rand() % size;				
			angle = AngleBetweenTwoLine(contours[random_1], contours[random_2]);
			++count;
			if (count > 10){
				break;
			}
		} while (angle > 45.0 || random_2 == random_1);

		cv::Point2d point;
		CalcIntersection(point, contours[random_1].point1, contours[random_1].point2,
			contours[random_2].point1, contours[random_2].point2);
//		ToString(contours[random_1]); ToString(contours[random_2]);
//		std::cout<< " intersection:" << point << endl;

		for (int j = 0; j < size; ++j){
			if (GoThroughPoint(point, contours[j])){
				++cur_count;
				cur_votes[j] = 1;
			}
		}
		if (best_count < cur_count){
			best_count = cur_count;
			vanishing_point = point;
			for (int k = 0; k < size; ++k){
				best_votes[k] = cur_votes[k];
			}
		}
	} */
	std::cout << "best count:" << best_count << std::endl;
}
void RansacForVanishPoint::Canny(cv::Mat& img_gray, 
	std::vector<Line>& vertical_contours,
	std::vector<Line>& horizontal_contours) {
	cv::Mat edge, dst;
	cv::blur(src_gray_, edge, cv::Size(5, 5));

	// Run the edge detector on grayscale
	cv::Canny(edge, edge, canny_thresh_, canny_thresh_ * 3, 3);
	cv::threshold(edge, edge, 10, 255, cv::THRESH_BINARY);
	
	//cv::Mat test1;
	//cv::blur(src_gray_, test1, cv::Size(5, 5));
	//// Run the edge detector on grayscale
	//cv::Canny(test1, test1, canny_thresh_, canny_thresh_ * 2, 3);
	//cv::namedWindow("threshold 2:1");
	//cv::imshow("threshold 2:1", test1);

	dst = edge;

	cv::setMouseCallback(*win_name_, delete_mouse_callback_static, this);
	cv::imshow(*win_name_, dst);

	//寻找直线边缘集, use HoughLines 
/*	vector<Vec2f> lines;
	HoughLines(dst, lines, 1, CV_PI / 180, hough_thresh_, 0, 0);

	cv::Mat drawing;
	dst.copyTo(drawing);
	cvtColor(drawing, drawing, CV_GRAY2BGR);
	//contours.clear();
	vertical_contours.clear();
	horizontal_contours.clear();

	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));

		Line temp_line;
		temp_line.point1 = pt1;
		temp_line.point2 = pt2;
		if (pt1.x == pt2.x)
		{
			temp_line.a = 0;
			temp_line.b = 0;
			temp_line.c = pt1.x;
		}
		else
		{
			temp_line.a = static_cast<double>(pt1.y - pt2.y) / (pt1.x - pt2.x);
			temp_line.b = pt1.y - temp_line.a * pt1.x;
			temp_line.c = 0;
		}
		
		if (temp_line.c != 0 || abs(temp_line.a) > min_vertical_k){
		    vertical_contours.push_back(temp_line);
		}
		else {
		    horizontal_contours.push_back(temp_line);
		}
		//contours.push_back(temp_line);
		line(drawing, pt1, pt2, Scalar(0, 0, 255), 1, CV_AA);
	}  */

	//寻找直线边缘集, use HoughLinesP
	vector<Vec4i> lines;
	cv::HoughLinesP(dst, lines, 1, CV_PI / 180, hough_thresh_, min_line_length_, max_line_gap_);

	//显示找到的直线集
	cv::Mat drawing;
	dst.copyTo(drawing);
	cvtColor(drawing, drawing, CV_GRAY2BGR);

	vertical_contours.clear();
	horizontal_contours.clear();
	for (int i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		Line temp_line;
		GetLine(l, temp_line);
		if (temp_line.c != 0 || abs(temp_line.a) > min_vertical_k){
			vertical_contours.push_back(temp_line);
		}
		else {
			horizontal_contours.push_back(temp_line);
		}
		
		line(drawing, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 255, 0), 1, CV_AA);
	}   

	cv::imshow(ALL_LINES_WINDOW, drawing);
	current_canny_edge_ = drawing;
}

void RansacForVanishPoint::GetEraseImg(const cv::Mat& origin, const cv::Mat& mask, cv::Mat _return){
	if (origin.empty() || origin.type() != CV_8UC1)
		CV_Error(Error::StsBadArg, "origin is empty or has incorrect type (not CV_8UC1)");
	if (_return.empty() || _return.rows != origin.rows || _return.cols != origin.cols)
		_return.create(origin.size(), CV_8UC1);
	origin.copyTo(_return, mask);
}

void RansacForVanishPoint::delete_mouse_callback(int event, int x, int y, int flags){
	switch (event)
	{
	case EVENT_LBUTTONDOWN:
	{
		if (rectState_ == NOT_SET)
		{
			rectState_ = IN_PROCESS;
			rect_ = Rect(x, y, 1, 1);
		}
	}
		break;
	case EVENT_LBUTTONUP:
		if (rectState_ == IN_PROCESS)
		{
			rect_ = Rect(Point(rect_.x, rect_.y), Point(x, y));
			rectState_ = SET;

			//mask.setTo(GC_BGD);
			rect_.x = max(0, rect_.x);
			rect_.y = max(0, rect_.y);
			rect_.width = min(rect_.width, src_gray_.cols - rect_.x);
			rect_.height = min(rect_.height, src_gray_.rows - rect_.y);
			(mask_(rect_)).setTo(Scalar(0));

			StartCalc();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState_ == IN_PROCESS)
		{
			rect_ = Rect(Point(rect_.x, rect_.y), Point(x, y));
			ShowCurrentChoose();
		}
	}
}

void RansacForVanishPoint::ShowCurrentChoose(){
	cv::Mat res;
	current_canny_edge_.copyTo(res);
	if (rectState_ == IN_PROCESS || rectState_ == SET)
		rectangle(res, Point(rect_.x, rect_.y), Point(rect_.x + rect_.width, rect_.y + rect_.height), cv::Scalar(0,0,255), 2);
	imshow(*win_name_, res);
}
void RansacForVanishPoint::GetLine(Vec4i line, Line& _return){
	_return.point1 = Point(line[0], line[1]);
	_return.point2 = Point(line[2], line[3]);
	if (line[0] == line[2])
	{
		_return.a = 0;
		_return.b = 0;
		_return.c = line[0];
	}
	else
	{
		_return.a = static_cast<double>(line[1] - line[3]) / (line[0] - line[2]);
		_return.b = line[1] - _return.a * line[0];
		_return.c = 0;
	}

}
