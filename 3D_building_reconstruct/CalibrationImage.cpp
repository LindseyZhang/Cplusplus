//#include <stdlib.h>
#include <iostream>
#include <opencv2\calib3d.hpp>
#include <math.h>
#include "CalibrationImage.h"
#include "common.h"


#define X_DIR_POINT_COLOR cv::Scalar(255,0,0)
#define Y_DIR_POINT_COLOR cv::Scalar(0,255,0)
#define Z_DIR_POINT_COLOR cv::Scalar(0,0,255)

cv::Mat m_src;
int m_threshold_x =0;
int m_threshold_y =0;
int m_threshold_z =0;


CalibrationImage::CalibrationImage(const std::string& picture_name)
{
	src_ = cv::imread(picture_name, cv::IMREAD_COLOR);
	if (!src_.data){
		std::cout << "could not find or open the image" << std::endl;
	}

	finish_ = false;
	world_origin_ = cv::Point2d(0, 0);
}

CalibrationImage::CalibrationImage(const std::string& src_name,cv::Mat& foreground)
{
	src_ = cv::imread(src_name, cv::IMREAD_COLOR);
	if (!src_.data){
		std::cout << "could not find or open the image" << std::endl;
	}
	foreground.copyTo(foreground_);
	background_ = src_ - foreground_;
	world_origin_ = cv::Point2d(0, 0);

}

CalibrationImage::~CalibrationImage()
{
}

void CalibrationImage::Help(){
	std::cout << "依次选择X方向，Y方向，Z方向的两条平行线的两个端点用于计算消失点\n" 
		"\nHot keys: \n"
		"\tn - start calc pseudo inverse matrix\n"
		"\tr - reset\n" 
		"\tg - use this matrix to do calibration\n"<< std::endl;
}

void CalibrationImage::Reset(){
	ClearTwoLinePointStruct(x_direction_);
	ClearTwoLinePointStruct(y_direction_);
	ClearTwoLinePointStruct(z_direction_);
	world_origin_ = cv::Point2d(0, 0);
	for (int i = 0; i < 3; ++i){
		vanishing_point_[i] = cv::Point2d(0, 0);
	}
	camera_matrix_.setTo(0);
	camera_f_ = 0;
	finish_ = false;
	std::cout << "already clear all the input" << std::endl;
}

void CalibrationImage::ClearTwoLinePointStruct(TwoLinePoints& stru){
	stru.all_filled = false;
	for (int i = 0; i < 4; ++i){
		stru.points[i].filled = false;
		stru.points[i].point = cv::Point2d(0, 0);
	}
}


void CalibrationImage::ThresholdCallback() {
	cv::Mat temp;
	// line in y direction
	cv::threshold(x_edge, temp, m_threshold_x, 255, cv::THRESH_BINARY);

	std::vector<cv::Vec4i> lines;
	int houghThreshold = 70;
	if (temp.cols*temp.rows < 400 * 400)
		houghThreshold = 100;

	cv::HoughLinesP(temp, lines, 1, CV_PI / 180, houghThreshold, 10, 3);

	while (lines.size() > 50)
	{
		lines.clear();
		houghThreshold += 10;
		cv::HoughLinesP(temp, lines, 1, CV_PI / 180, houghThreshold, 10, 3);
	}

	double line_length1 = 0, line_length2 = 0;
	for (size_t i = 0; i < lines.size(); i++)
	{
		Line line;
		line.point1.x = lines[i][0];
		line.point1.y = lines[i][1];
		line.point2.x = lines[i][2];
		line.point2.y = lines[i][3];
		double length = EculidDistance_2d(line.point1, line.point2);
		if (line_length1 < length){
			y_lines[0] = line;
			line_length1 = length;
		}else if (line_length2 < length){
			y_lines[1] = line;
			line_length2 = length;
		}

	//	cv::line(m_src, line.point1, line.point2, cv::Scalar(255, 0, 0), 2);
	}
	
	cv::line(m_src, y_lines[0].point1, y_lines[0].point2, cv::Scalar(0, 255, 0), 2);
	cv::line(m_src, y_lines[1].point1, y_lines[1].point2, cv::Scalar(0, 255, 0), 2);

	//line in x direction
	temp.setTo(0);
	cv::threshold(y_edge, temp, m_threshold_y, 255, cv::THRESH_BINARY);

	lines.clear();
	cv::HoughLinesP(temp, lines, 1, CV_PI / 180, houghThreshold, 10, 3);

	while (lines.size() > 50)
	{
		lines.clear();
		houghThreshold += 10;
		cv::HoughLinesP(temp, lines, 1, CV_PI / 180, houghThreshold, 10, 3);
	}

	line_length1 = 0;
	line_length2 = 0;
	for (size_t i = 0; i < lines.size(); i++)
	{
		Line line;
		line.point1.x = lines[i][0];
		line.point1.y = lines[i][1];
		line.point2.x = lines[i][2];
		line.point2.y = lines[i][3];
		double length = EculidDistance_2d(line.point1, line.point2);
		if (line_length1 < length){
			x_lines[0] = line;
			line_length1 = length;
		}
		else if (line_length2 < length){
			x_lines[1] = line;
			line_length2 = length;
		}
		//	cv::line(m_src, line.point1, line.point2, cv::Scalar(255, 0, 0), 2);
	}

	cv::line(m_src, x_lines[0].point1, x_lines[0].point2, cv::Scalar(255, 0, 0), 2);
	cv::line(m_src, x_lines[1].point1, x_lines[1].point2, cv::Scalar(255, 0, 0), 2);
	ThresholdCallback();

	cv::imshow("x_edge", temp);
	cv::namedWindow("drawing");
	cv::imshow("drawing", m_src);
	
}

void CalibrationImage::FindEdges() {
	src_.copyTo(m_src);
	cv::Mat gray, edge,cedge;
	int edgeThresh = 100;
	gray.create(src_.size(), src_.type());
	cv::cvtColor(src_, gray, CV_BGR2GRAY);
	cv::blur(gray, edge, cv::Size(3, 3));
	cv::namedWindow("x_edge");
	cv::namedWindow("y_edge");

	// Run the edge detector on grayscale
	//cv::Canny(edge, edge, edgeThresh, edgeThresh * 3, 3);
	//cedge = cv::Scalar::all(1);
	//
	//gray.copyTo(cedge, edge);
	cv::Sobel(edge, x_edge, -1, 1, 0, 3);
	cv::Sobel(edge, y_edge, -1, 0, 1, 3);
	cv::createTrackbar("threshold", "x_edge", &m_threshold_x, 255, &CalibrationImage::OnChange,this);
	cv::createTrackbar("threshold", "y_edge", &m_threshold_y, 255, &CalibrationImage::OnChange, this);

	cv::imshow("x_edge",x_edge);
	cv::imshow("y_edge", y_edge);
	cv::waitKey();
}


bool CalibrationImage::CalcOrthocenter(cv::Point2d& _return, cv::Point2d points[3]){
	//先求两条高线，其交点就是垂心M
	// 根据向量BC*向量AM = 0，向量AC*向量BM = 0
	//化简后得（Xb-Xc)*x + (Yb-Yc)*y= Xa*(Xb-Xc) + Ya*(Yb-Yc) 
	//（Xa-Xc)*x + (Ya-Yc)*y= Xb*(Xa-Xc) + Ya*(Yb-Yc）
	Line l1, l2;
	GetPerpendicularLine(l1,points[1], points[2], points[0]);
	GetPerpendicularLine(l2,points[0], points[2], points[1]);
	//a0*x + b0*y + c0 = a1*x + b1*y + c1
	//	由此可推出
	//	x = (b0*c1 – b1*c0) / D
	//	y = (a1*c0 – a0*c1) / D
	//	D = a0*b1 – a1*b0，(D为0时，表示两直线重合)

	double d = l1.a*l2.b - l2.a*l1.b;
	if (fabs(d) < PRECISION){
		return false;
	}
	_return.x = (l1.b*l2.c - l2.b*l1.c) / d;
	_return.y = (l2.a*l1.c - l1.a*l2.c) / d;
	return true;

}

//（Xb-Xc)*x + (Yb-Yc)*y + Xa*(Xc-Xb) + Ya*(Yc-Yb) = 0  p1 对应Xb,p2 对应Xc,p3对应Xa
inline void CalibrationImage::GetPerpendicularLine(Line& line, cv::Point2d p1, cv::Point2d p2,cv::Point2d p3){
	line.a = p1.x - p2.x;
	line.b = p1.y - p2.y;
	line.c = p3.x*(p2.x - p1.x) + p3.y*(p2.y - p1.y);
}


bool CalibrationImage::SetCameraMatrix(cv::Point2d points[3]){
	cv::Point2d orthocenter, foot; //垂心，垂足
	CalcOrthocenter(orthocenter, points);
	//垂心为相机主点
	principle_point_ = orthocenter;
	//该方法实际为求两直线交点，用来求垂足
	//垂足在points[0],points[1] 边上
/*	CalcVanishingPoint(foot, points[0], points[1], points[2], orthocenter);

	double temp1 = EculidDistance_2d(points[0], foot);
	double temp2 = EculidDistance_2d(points[1], foot);
	double temp3 = EculidDistance_2d(orthocenter, foot);

	double f = sqrt(temp1*temp2 - temp3*temp3);  */
	//求焦距
	double temp1 = (points[0].x - orthocenter.x)*(points[1].x - orthocenter.x);
	double temp2 = (points[0].y - orthocenter.y)*(points[1].y - orthocenter.y);
	
	
	double temp_res = -temp1 - temp2;
	if (temp_res < 0.0) {
		std::cout << std::endl << std::endl;
		if (IS_TEST) {
			std::cout << "temp1:" << temp1 << "  temp2:" << temp2 << std::endl;
			std::cout << "f = sqrt(-temp1 - tmep2）" << std::endl;
			std::cout << "-temp1 - temp2 = " << temp_res << std::endl;
		}
		std::cout << "WARING:vanishing point might be wrong, can't get camera focus.\n please calculate them again\n " << std::endl;
		return false;
	}
	double f = sqrt(temp_res);
	std::cout << " f = sqrt(-temp1 - temp2)= " << f << std::endl;
	camera_f_ = f;

	camera_matrix_.at<double>(0, 0) = f;
	camera_matrix_.at<double>(0, 2) = orthocenter.x;
	camera_matrix_.at<double>(1, 1) = f;
	camera_matrix_.at<double>(1, 2) = orthocenter.y;
	camera_matrix_.at<double>(2, 2) = 1;

	std::cout << "camera_matrix: " << std::endl << camera_matrix_ << std::endl;
	return true;
}


inline void CalibrationImage::GetVectorInCameraCoordinateSystem(cv::Point3d& _return,cv::Point2d start, cv::Point2d end){
	_return.x = end.x - start.x;
	_return.y = end.y - start.y;
	_return.z = camera_f_;
}
void CalibrationImage::CalcExtrinsicMatrix(cv::Mat& _return,cv::Point2d vanishing_points[3]){
	std::cout << "starting calc extrinsic matrix..." << std::endl;
	//calc rotation matrix
	cv::Point3d Vx, Vy;
	GetVectorInCameraCoordinateSystem(Vx, principle_point_, vanishing_points[0]);
	GetVectorInCameraCoordinateSystem(Vy, principle_point_, vanishing_points[1]);

	cv::Point3d origin(0, 0, 0);
	double Vx_length = EculidDistance_3d(Vx, origin);
	double Vy_length = EculidDistance_3d(Vy, origin);

	Vx = Vx / Vx_length;
	Vy = Vy / Vy_length;

	//Vz为Vx,Vy 的叉乘
	cv::Point3d Vz;
	Vz.x = Vx.y*Vy.z - Vx.z*Vy.y;
	Vz.y = Vx.z*Vy.x - Vx.x*Vy.z;
	Vz.z = Vx.x*Vy.y - Vx.y*Vy.x;

	SetRotaionMatByColunm(_return, 0, Vx);
	SetRotaionMatByColunm(_return, 1, Vy);
	SetRotaionMatByColunm(_return, 2, Vz);


	//转置矩阵，用户选取的原点与世界坐标系的原点对应
	cv::Mat mat1 = (cv::Mat_<double>(3,1) << world_origin_.x, world_origin_.y, camera_f_);
	std::cout << "world_origin position:" << mat1;
	cv::Mat cam_matrix(camera_matrix_, cv::Rect(0, 0, 3, 3));
	cv::Mat camera_matrix_invert = cv::Mat::zeros(cv::Size(3, 3), CV_64F);
	double ret = cv::invert(cam_matrix, camera_matrix_invert, cv::DECOMP_LU);
	if (!ret) {
		std::cout << "matrix is singular" << std::endl;
	}
	cv::Mat Trans = camera_matrix_invert*mat1;
	cv::Point3d T(Trans.at<double>(0,0), Trans.at<double>(1,0), Trans.at<double>(2,0));
	SetRotaionMatByColunm(_return, 3, T);

	_return.at<double>(3, 3) = 1;
	std::cout << "extrinsic matrix:" << std::endl << _return << std::endl;
}



void CalibrationImage::SetRotaionMatByColunm(cv::Mat& mat, int column, cv::Point3d vector){
	mat.at<double>(0, column) = vector.x;
	mat.at<double>(1, column) = vector.y ;
	mat.at<double>(2, column) = vector.z ;
}
cv::Mat CalibrationImage::GetPseudoInverseMatrix(){
	return pseudo_inverse_matrix_;
}

void CalibrationImage::CalcPseudoInverseMatrix(){
	cv::Mat projection_matrix = cv::Mat::zeros(cv::Size(3,4),CV_64F);
	cv::Mat temp = cv::Mat::zeros(cv::Size(3, 4), CV_64F);
//	cv::gemm(camera_matrix_, extrinsic_matrix_, 1, temp, 0,projection_matrix);
	projection_matrix = camera_matrix_* extrinsic_matrix_;

	cv::Mat transpose_projection_matrix = cv::Mat::zeros(cv::Size(4, 3), CV_64F);
	cv::transpose(projection_matrix, transpose_projection_matrix);
	std::cout << " transpose projection_matrix:" << std::endl << transpose_projection_matrix << std::endl;

	cv::Mat temp_result = cv::Mat::zeros(cv::Size(4, 4), CV_64F);
	cv::Mat temp_result_invert = cv::Mat::zeros(cv::Size(4, 4), CV_64F);
//	cv::gemm(transpose_projection_matrix, projection_matrix, 1, temp, 0, temp_result);
	temp_result = transpose_projection_matrix * projection_matrix;
	cv::invert(temp_result, temp_result_invert, cv::DECOMP_LU);
//	std::cout << "temp:" << temp_result_invert << std::endl;

	pseudo_inverse_matrix_ = temp_result_invert*transpose_projection_matrix;
	std::cout << "pseudo_inverse_matrix:" << std::endl << pseudo_inverse_matrix_ << std::endl;
}

void CalibrationImage::ShowSrc(){	
	if (src_.empty()) {
		std::cout << "ShowImage: image is empty " << std::endl;
		return;
	}

	cv::Mat res;
	src_.copyTo(res);


	for (int i = 0; i < 4; ++i) {
		if (x_direction_.points[i].filled){
			cv::circle(res, x_direction_.points[i].point, radius, X_DIR_POINT_COLOR, thickness);
		}
		if (y_direction_.points[i].filled){
			cv::circle(res, y_direction_.points[i].point, radius, Y_DIR_POINT_COLOR, thickness);
		}
		if (z_direction_.points[i].filled){
			cv::circle(res, z_direction_.points[i].point, radius, Z_DIR_POINT_COLOR, thickness);
		}
	}
	imshow(CALIBRATION_IMAGE_WINDOW_NAME, res);
}

void CalibrationImage::ChooseWorldOrigin(int x, int y){
	std::cout << "shoose world origin:(" << x << ","
		<< y << ")" << std::endl;
	world_origin_.x = x;
	world_origin_.y = y;
}

bool CalibrationImage::IsFinish(){
	return finish_;
}

bool CalibrationImage::SetPoint(DIRECTION dir,int x, int y){
	switch (dir){
	case X: 
		if (x_direction_.all_filled) return true;
		for (int i = 0; i < 4; i++){
			if (!x_direction_.points[i].filled){ 
				x_direction_.points[i].point.x = x;
				x_direction_.points[i].point.y = y;
				x_direction_.points[i].filled = true;
				std::cout << "set point x:" << x_direction_.points[i].point << std::endl;
				if (3 == i){
					x_direction_.all_filled = true;
					return true;
				} else {
					return false;
				}
			}
		}
		return true;
	case Y:
		if (y_direction_.all_filled) return true;
		for (int i = 0; i < 4; i++){
			if (!y_direction_.points[i].filled){
				y_direction_.points[i].point.x = x;
				y_direction_.points[i].point.y = y;
				y_direction_.points[i].filled = true;
				std::cout << "set point y:" << y_direction_.points[i].point << std::endl;
				if (3 == i){
					y_direction_.all_filled = true;
					return true;
				}
				else {
					return false;
				}
			}
		}
		return true;
	case Z:
		if (z_direction_.all_filled) return true;
		for (int i = 0; i < 4; i++){
			if (!z_direction_.points[i].filled){
				z_direction_.points[i].point.x = x;
				z_direction_.points[i].point.y = y;
				z_direction_.points[i].filled = true;
				std::cout << "set point z:" << z_direction_.points[i].point << std::endl;
				if (3 == i){
					z_direction_.all_filled = true;
					return true;
				}
				else {
					return false;
				}
			}
		}
		return true;
	default:
		return false;
	}
}


bool CalibrationImage::LinePointAllSet(DIRECTION dir){
	switch (dir){
	case X: return x_direction_.all_filled;
	case Y: return y_direction_.all_filled;
	case Z: return z_direction_.all_filled;
	default: std::cout << "wrong direction parameter" << std::endl;
		return false;
	}
}

void CalibrationImage::SetVanishingPoint(cv::Point2d vanishing_point[3]){
	for (unsigned i = 0; i < 3; ++i){
		vanishing_point_[i] = vanishing_point[i];
	}
}

void CalibrationImage::SetVanishingPoint(){
	//vanishing point
	CalcIntersection(vanishing_point_[0], x_direction_.points[0].point,
		x_direction_.points[1].point,
		x_direction_.points[2].point,
		x_direction_.points[3].point);
	CalcIntersection(vanishing_point_[1], y_direction_.points[0].point,
		y_direction_.points[1].point,
		y_direction_.points[2].point,
		y_direction_.points[3].point);
	CalcIntersection(vanishing_point_[2], z_direction_.points[0].point,
		z_direction_.points[1].point,
		z_direction_.points[2].point,
		z_direction_.points[3].point);
	std::cout << "x direction vanishing point:" << vanishing_point_[0] << std::endl
		<< "y direction vanishing point:" << vanishing_point_[1] << std::endl
		<< "z direction vanishing point:" << vanishing_point_[2] << std::endl;
}

bool CalibrationImage::SetCalibrationMatrix(){

	if (!SetCameraMatrix(vanishing_point_)) {
		return false;
	}
	CalcExtrinsicMatrix(extrinsic_matrix_, vanishing_point_);
	CalcPseudoInverseMatrix();
	//for (unsigned i = 0; i <4; ++i){
	//	cv::Point2d point_2d = x_direction_.points[i].point;		
	//	cv::Point3d point_3d;
	//	//CalibratePoint(point_2d, point_3d);
	//	CalibratePoint(point_2d, &pseudo_inverse_matrix_, point_3d);
	//	std::cout << "2 dimension point:" << std::endl << point_2d << std::endl <<
	//		"3 dimension point:" << std::endl << point_3d << std::endl;
	//}
	finish_ = true;
	std::cout << "finish calc pseudo inverse matrix" << std::endl;
	return true;
}


void CalibrationImage::ChangePointTo3dinOpenGL(std::vector<PointInfo>& points_in_opengl,int* ortho,int* rows,int* cols){
	*rows = src_.rows;
	*cols = src_.cols;
	for (int i = 0; i < src_.rows; ++i){
		for (int j = 0; j < src_.cols; ++j){
			PointInfo temp;
			cv::Point2d point(i, j);
			cv::Point3d out_point;
			//CalibratePoint(point, out_point);
			CalibratePoint(point, &pseudo_inverse_matrix_, out_point);
			if (out_point.y < ortho[0]){
				ortho[0] = out_point.y;
			}
			if (out_point.y > ortho[1]){				
				ortho[1] = out_point.y;
			}
			if (out_point.x > ortho[2]){
				ortho[2] = out_point.x;
			}
			if (out_point.x < ortho[3]){
				ortho[3] = out_point.x;
			}
			if (out_point.z < ortho[4]){
				ortho[4] = out_point.z;
			}
			if (out_point.z > ortho[5]){
				ortho[5] = out_point.z;
			} 

			temp.position[0] = out_point.y;
			temp.position[1] = -out_point.x;
			temp.position[2] = out_point.z;

			cv::Point3d p = src_.at<cv::Vec3b>(i, j);
			temp.color[0] = p.z / 255;   //BGR change to RGB
			temp.color[1] = p.y / 255; 
			temp.color[2] = p.x / 255;
			points_in_opengl.push_back(temp);
		}
	}
	std::cout << "\n in func change:  ortho:" << ortho[0] << "," << ortho[1] << "," << ortho[2] << "," << ortho[3] << "," << ortho[4] << "," << ortho[5] << std::endl;
}