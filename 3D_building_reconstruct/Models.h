#pragma once
#include "common.h"
#include <vector>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <gl/glut.h>



enum MODE {
	SET_X,
	SET_Y,
	SET_Z,
	SET_ORIGIN,
	SET_XY_ACROSS_CORNER,
	SET_YZ_ACROSS_CORNER,
	//ADD_FRONT_TEXTURE,
	//ADD_SIDE_TEXTURE,
	NONE,
};

class Models
{
public:
	Models();
	~Models();
	void help();
	void DrawCubes();
	void AddCube(int* ortho);
	void ShowImage();
	void ShowModels();
	bool SetImageWindowAndMatrix(const cv::Mat& _image, 
		const cv::Mat& foreground, 
		const std::string& _winName, 
		const cv::Mat& matrix);
private:
	static void model_mouse_callback(int event, int x, int y, int flags, void* param);
	void DrawCube(Cube& cube);
	void ShowMarkedPoints(Cube* cube);
	void PushBackCube(Cube& cube, int* ortho);
	void GenerateTexture(Cube& cube);
	/**
	four_corners   待矫正四边形从左下角开始各顶点的逆时针队列
	width   矫正后图片的宽度
	height  矫正后图片的高度
	textureID  生成的纹理ID
	*/
	void GenerateTextureByDetail(
		std::vector<cv::Point2d>& four_corners,
		double width, 
		double height, 
		GLuint* textureID);

	bool CheckCubeOk(Cube& cube);

	std::vector<Cube> cube_list_;

	static const int radius = 2;
	static const int thickness = -1;

	const std::string* win_name_;
	const cv::Mat* src_;
	const cv::Mat* foreground_;
	const cv::Mat* pseudo_inverse_matrix_;

	MODE mode_;

};


