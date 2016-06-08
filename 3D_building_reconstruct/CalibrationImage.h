#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>
#include "common.h"
#include <gl/glut.h>

#define PRECISION 0.000001

enum DIRECTION {
	X = 1,
	Y = 2,
	Z = 3
};

struct SelectedPoint{
	bool filled = false;
	cv::Point2d point;
};
struct TwoLinePoints{
	SelectedPoint points[4];
	bool all_filled = false;
};

//先find corner, then findSubPix

class CalibrationImage
{
public:
	CalibrationImage(const std::string& filename);
	CalibrationImage(const std::string& src_name,cv::Mat& foreground);
	~CalibrationImage();

	void Help();
	void FindEdges();
	/** 用于计算伪逆成像矩阵， 需要先调用SetVanishingPoint计算消失点*/
	bool SetCalibrationMatrix();
	/**给消失点赋值*/
	void SetVanishingPoint(cv::Point2d vanishing_point[3]);
	/** 用于用户交互时计算消失点*/
	void SetVanishingPoint();

	void CalcExtrinsicMatrix(cv::Mat& _return,cv::Point2d vanishing_points[3]);
	cv::Mat GetPseudoInverseMatrix();
	void ShowSrc();
	bool SetPoint(DIRECTION dir,int x, int y);
	
	bool LinePointAllSet(DIRECTION dir);
	void ChangePointTo3dinOpenGL(std::vector<PointInfo>& points_in_opengl, int* ortho, int* rows, int* cols);
	void ChooseWorldOrigin(int x,int y);
	bool IsFinish();
	void Reset();
private:	
	//_return 为垂心， points数组为三角形的三个顶点
	bool CalcOrthocenter(cv::Point2d& _return, cv::Point2d points[3]);
	/** 根据成像矩阵和外参矩阵计算伪逆成像矩阵*/
	void CalcPseudoInverseMatrix();
	/**参数为互相垂直的三个方向的灭点*/
	bool SetCameraMatrix(cv::Point2d vanishing_points[3]);

	static void OnChange(int, void* object){
		CalibrationImage* ptr = (CalibrationImage*)object;
		ptr->ThresholdCallback();
	}
	void ThresholdCallback();
	inline void GetPerpendicularLine(Line& line, cv::Point2d p1, cv::Point2d p2,cv::Point2d p3);
	inline void GetVectorInCameraCoordinateSystem(cv::Point3d& _return,cv::Point2d start, cv::Point2d end);
	void SetRotaionMatByColunm(cv::Mat& mat, int column, cv::Point3d vector);
	void ClearTwoLinePointStruct(TwoLinePoints& stru);

	//以下三个变量在调用SetCameraMatrix后赋值
	cv::Mat camera_matrix_ = cv::Mat::zeros(cv::Size(4,3),CV_64F);	
	cv::Point2d principle_point_;  //相机主点	
	double camera_f_;

	cv::Point2d vanishing_point_[3];   //消失点，依次为x,y,z方向
	cv::Mat extrinsic_matrix_ = cv::Mat::zeros(cv::Size(4, 4), CV_64F);
	cv::Mat pseudo_inverse_matrix_ = cv::Mat::zeros(4,3,CV_64F);
	cv::Mat dist_coeffs_;
	std::vector<std::vector<cv::Point2d> > image_points_;
	std::vector<std::string> image_list_;
	std::vector<cv::Point2d> pointbuf_;
	cv::Mat src_;
	TwoLinePoints x_direction_, y_direction_, z_direction_;
	Line x_lines[2], y_lines[2], z_lines[2];
	cv::Mat x_edge, y_edge;
	cv::Mat foreground_, background_;
	cv::Point2d world_origin_;
	bool finish_;

	static const int radius = 2;
	static const int thickness = -1;
};

