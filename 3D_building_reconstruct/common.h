#ifndef BUILDING_RECONSTRUCT_COMMON_H
#define BUILDING_RECONSTRUCT_COMMON_H

#include <opencv2/core/core.hpp>
#include <vector>
#include <gl/glut.h>
 
#define GRABCUT_SRC_WINDOW_NAME "grabcut to extract foreground"
#define GL_MAIN_WINDOW_NAME "3D building reconstruction demo"
#define CALIBRATION_IMAGE_WINDOW_NAME "calibration image"
#define CHOOSE_MODEL_WINDOW_NAME "choose model"

//auto vanishing point calc
#define TRACKBAR_WINDOW	"track_bar"
#define LINES_USED_TO_CALC_VANISHING_WINDOW "uesd lines"
#define ALL_LINES_WINDOW "all lines find"

#define IS_TEST false

enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };

struct PointInfo{
	GLfloat position[3];
	double color[3];
};

struct Line{
	//y = a*x + b  or x = c
	cv::Point2d point1, point2;
	double a = 0, b = 0, c = 0;
};

struct Cube {
	int z_width;  //长宽高
	int y_height;
	int x_long;
	GLdouble world_position[3];  //前面左下角在世界坐标系中的位置  
	GLdouble point[6][2];  //分别对应x,y,z方向的端点,模型原点,和xy平面对角点，yx平面对角点
	GLuint texture[3] ; //依次对应正面，侧面和上面的纹理
//	std::vector<cv::Point> texture_point[3];
	Cube()  {
		for (int i = 0; i < 6; ++i){
			point[i][0] = -1; point[i][1] = -1;
		}		
		world_position[0] = -1; world_position[1] = -1; world_position[2] = -1;
		for (int i = 0; i < 3; ++i){
			texture[i] = 0;
		}
	}
};

template<class T>
struct Param {
	Cube* cube_ptr;
	T* class_ptr;
};

//void Inverse2DTo3D(GLdouble in_point[2], const cv::Mat* inverse_matrix,GLdouble* out_point[3]) {
//	cv::Mat mat_2d = (cv::Mat_<double>(3, 1) << in_point[0],in_point[1],1);
//	cv::Mat mat_3d = cv::Mat::zeros(4, 1, CV_64F);
//
//	mat_3d = *inverse_matrix * mat_2d;
//
//	double last = mat_3d.at<double>(3, 0);
//	*out_point[0] = mat_3d.at<double>(0, 0) / last;
//	*out_point[1] = mat_3d.at<double>(1, 0) / last;
//	*out_point[2] = mat_3d.at<double>(2, 0) / last;
//}

void CalibratePoint(cv::Point2d& in_point, const cv::Mat* pseudo_inverse_matrix, cv::Point3d& out_point);

double EculidDistance_2d(cv::Point2d p1, cv::Point2d p2);

double EculidDistance_3d(cv::Point3d p1, cv::Point3d p2);

//计算两直线交点
bool CalcIntersection(
	cv::Point2d& _return, 
	cv::Point2d line1_p1,
	cv::Point2d line1_p2, 
	cv::Point2d line2_p1, 
	cv::Point2d line2_p2);

//判断点p是否通过直线line
bool GoThroughPoint(cv::Point2d p, Line& line);


void ToString(Line& line);

int PowOf2(int value);

double AngleBetweenTwoLine(Line& line1, Line& line2);

#endif