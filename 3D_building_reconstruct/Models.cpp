#include "Models.h"
#include "common.h"
#include "TextureExtrator.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <gl/glut.h>

#define ORIGIN_COLOR cv::Scalar(0,0,0)
#define X_END_COLOR cv::Scalar(255,0,0)
#define Y_END_COLOR cv::Scalar(0,255,0)
#define Z_END_COLOR cv::Scalar(0,0,255)
#define ACROSS_CORNER_COLOR cv::Scalar(255,255,0)


// cube model's normal , face and end_point
GLfloat n[6][3] = {  /* Normals for the 6 faces of a cube. */
		{ 0.0, 1.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 0.0, 0.0, -1.0 }, 
		{ 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0 }, { -1.0, 0.0, 0.0 }}; 
//face order: top,down, back,front,right,left
//each face is anticlockwise, left bottom, right bottom, right top, left top
GLint faces[6][4] = {  /* Vertex indices for the 6 faces of a cube. */
		{ 0, 1, 2, 3 }, {  7, 6,5,4 }, { 6, 7, 3, 2 },
		{ 4, 5, 1, 0 }, { 5, 6, 2, 1 }, { 7, 4, 0, 3 } };
GLfloat g_vertices[8][3];

Models::Models()
{
	mode_ = NONE;
}


Models::~Models()
{
}

bool Models::SetImageWindowAndMatrix(const cv::Mat& _image,
	const cv::Mat& foreground, 
	const std::string& _winName, 
	const cv::Mat& matrix){
	if (_image.empty()) {
		std::cout << "SetImageAndWinName: _image is empty " << std::endl;
		return false;
	}
	if (foreground.empty()) {
		std::cout << "SetImageAndWinName: foreground is empty " << std::endl;
		return false;
	}
	if (_winName.empty()) {
		std::cout << "SetImageAndWinName:  _winName is empty" << std::endl;
		return false;
	}
	if (matrix.empty()) {
		std::cout << "SetImageAndWinName:  matrix is empty" << std::endl;
		return false;
	}
	src_ = &_image;
	foreground_ = &foreground;
	win_name_ = &_winName;
	pseudo_inverse_matrix_ = &matrix;
	return true;
}

void Models::help() {
	std::cout << "\nfollow the instruction, choose a model and mark the feature points.\n"
		"\nHot keys: \n"
		"\tr - reset\n"
		"\ta - add model (after set all below parameter)\n"
		"\n"
		"\to - set model origin point\n"
		"\tx - change to set the x direction end point mode\n"
		"\ty - change to set the y direction end point mode\n"
		"\tz - change to set the z direction end point mode\n"
		"\t1 - add cross corner of rectangle on plane xoy\n"
		"\t2 - add cross conner of rectangle on plane zoy\n" << std::endl;

}

//长,宽,高分别是x, z,y, 物体表示点为前面左下角
void Models::DrawCube(Cube& cube) {
	double x = cube.x_long;
	double y = cube.y_height;
	double z = cube.z_width;
	g_vertices[0][2] = g_vertices[1][2] = g_vertices[4][2] = g_vertices[5][2] = 0;
	g_vertices[2][2] = g_vertices[3][2] = g_vertices[6][2] = g_vertices[7][2] = -z;
	g_vertices[0][1] = g_vertices[1][1] = g_vertices[2][1] = g_vertices[3][1] = y;
	g_vertices[4][1] = g_vertices[5][1] = g_vertices[6][1] = g_vertices[7][1] = 0;
	g_vertices[1][0] = g_vertices[2][0] = g_vertices[5][0] = g_vertices[6][0] = x;
	g_vertices[3][0] = g_vertices[4][0] = g_vertices[8][0] = g_vertices[7][0] = 0;

	for (int i = 0; i < 6; i++){

		if (i == 0 || i == 1){
			glBindTexture(GL_TEXTURE_2D, cube.texture[2]);
		} else if (i == 2 || i == 3){
			glBindTexture(GL_TEXTURE_2D, cube.texture[0]);
		} else if (i == 4 || i == 5){
			glBindTexture(GL_TEXTURE_2D, cube.texture[1]);
		} 
		glBegin(GL_QUADS);
		glColor3f(1.0f, 1.0f, 1.0f);
		glNormal3fv(&n[i][0]);
		//left bottom, right bottom, right top, left top
		glTexCoord2f(0.0f, 0.0f);
		glVertex3fv(&g_vertices[faces[i][0]][0]);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3fv(&g_vertices[faces[i][1]][0]);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3fv(&g_vertices[faces[i][2]][0]);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3fv(&g_vertices[faces[i][3]][0]);
		glEnd();
	}
}

void Models::DrawCubes(){
	for (unsigned i = 0; i < cube_list_.size(); ++i) {
		glTranslated(cube_list_[i].world_position[0],
			cube_list_[i].world_position[1],
			cube_list_[i].world_position[2]);
		DrawCube(cube_list_[i]);
	}
}

void Models::AddCube(int* ortho){
	Cube cube;
	Param<Models> param;
	param.cube_ptr = &cube;
	param.class_ptr = this;
	cv::setMouseCallback(*win_name_, model_mouse_callback, reinterpret_cast<void*>(&param));
	ShowMarkedPoints(&cube);
	while (1) {
		int c = cv::waitKey(0);
		switch ((char)c) {
		case 'o':
		case 'O':
			mode_ = SET_ORIGIN;
			//	help();
			break;
		case 'x':
		case 'X':
			mode_ = SET_X;
			break;
		case 'y':
		case 'Y':
			mode_ = SET_Y;
			break;
		case 'z':
		case 'Z':
			mode_ = SET_Z;
			break;
		case 'r':
		case 'R':
			mode_ = NONE;
			cube.point[0][0] = -1; cube.point[0][1] = -1;
			cube.point[1][0] = -1; cube.point[1][1] = -1;
			cube.point[2][0] = -1; cube.point[2][1] = -1;
			for (int i = 0; i < 3; ++i){
				cube.texture[i] = NULL;
			}
			cv::imshow(*win_name_, *src_);
			break;
		case '1':{
			mode_ = SET_XY_ACROSS_CORNER;
			break;
		}
		case '2':
			mode_ = SET_YZ_ACROSS_CORNER;
			break;
		case 'a':
		case 'A':
			PushBackCube(cube, ortho);
			return;
		default:
			std::cout << "wrong command,input again." << std::endl;
			help();
		}
	}
}

bool Models::CheckCubeOk(Cube& cube){

	if (cube.point[0][0] == -1 || cube.point[0][1] == -1){
		std::cout << "add cube model failed. please set x end point." << std::endl;
		return false;
	}
	if (cube.point[1][0] == -1 || cube.point[1][1] == -1){
		std::cout << "add cube model failed. please set y end point." << std::endl;
		return false;
	}
	if (cube.point[2][0] == -1 || cube.point[2][1] == -1){
		std::cout << "add cube model failed. please set z end point." << std::endl;
		return false;
	}
	if (cube.point[3][0] == -1 || cube.point[3][1] == -1){
		std::cout << "add cube model failed. please set origin point." << std::endl;
		return false;
	}
	if (cube.point[4][0] == -1 || cube.point[4][1] == -1){
		std::cout << "add cube model failed. please set acorss corner of plane xoy." << std::endl;
		return false;
	}
	if (cube.point[5][0] == -1 || cube.point[5][1] == -1){
		std::cout << "add cube model failed.please set acorss corner of plane yoz." << std::endl;
		return false;
	}

	return true;
}
void Models::PushBackCube(Cube& cube, int* ortho) {
	if (!CheckCubeOk(cube)){
		return;
	}
	//计算模型长宽高
	cv::Point2d point_origin(cube.point[3][0], cube.point[3][1]);
	cv::Point2d point_x(cube.point[0][0], cube.point[0][1]);
	cv::Point2d point_y(cube.point[1][0], cube.point[1][1]);
	cv::Point2d point_z(cube.point[2][0], cube.point[2][1]);
	cv::Point3d origin_3d, x_3d, y_3d, z_3d;

	CalibratePoint(point_origin, pseudo_inverse_matrix_, origin_3d);
	CalibratePoint(point_x, pseudo_inverse_matrix_, x_3d);
	CalibratePoint(point_y, pseudo_inverse_matrix_, y_3d);
	CalibratePoint(point_z, pseudo_inverse_matrix_, z_3d);

	cube.x_long = EculidDistance_3d(x_3d, origin_3d);
	cube.y_height = EculidDistance_3d(y_3d, origin_3d);
	cube.z_width = EculidDistance_3d(z_3d, origin_3d);

	cube.world_position[0] = origin_3d.x;
	cube.world_position[1] = origin_3d.y;
	cube.world_position[2] = origin_3d.z;

	//generate cube texture	
	GenerateTexture(cube);

	//更新视窗大小
	double temp;
	double scalar = 3;
	temp = (cube.world_position[0] - cube.x_long) * scalar;
	if (temp < ortho[0]){
		ortho[0] = temp;
	}
	temp = (cube.world_position[0] + cube.x_long) * scalar;
	if (temp > ortho[1]){
		ortho[1] = temp;
	}
	temp = (cube.world_position[1] - cube.y_height) * scalar;
	if (temp < ortho[2]){
		ortho[2] = temp;
	}
	temp = (cube.world_position[1] + cube.y_height) * scalar;
	if (temp > ortho[3]){
		ortho[3] = temp;
	}
	temp = (cube.world_position[2] - cube.z_width) * scalar;
	if (temp < ortho[4]){
		ortho[4] = temp;
	}
	temp = (cube.world_position[2] + cube.z_width) * scalar;
	if (temp > ortho[5]){
		ortho[5] = temp;
	}

	std::cout << "add cube model:x=" << cube.x_long 
	<< " y=" << cube.y_height 
	<< " z=" << cube.z_width;
	std::cout << " origin: (" << cube.world_position[0] 
	<< "," << cube.world_position[1]
		<< "," << cube.world_position[2] << ")" << std::endl;
	cube_list_.push_back(cube);
}

void Models::GenerateTexture(Cube& cube){

	std::cout << "before generate texture:" 
	<< cube.texture[0] << std::endl;
	//generate xoy texture
	std::vector<cv::Point2d> four_corners;
	four_corners.push_back(	cv::Point2d(cube.point[0][0],
			       	cube.point[0][1]));  //x end
	four_corners.push_back(cv::Point2d(cube.point[3][0], 
				cube.point[3][1]));  //origin
	four_corners.push_back(	cv::Point2d(cube.point[1][0],
			       	cube.point[1][1]));   //y end
	four_corners.push_back(cv::Point2d(cube.point[4][0], 
				cube.point[4][1]));   //xoy across corner
	GenerateTextureByDetail(
			four_corners,
		       	cube.x_long,
		       	cube.y_height, 
			&(cube.texture[0]));
	std::cout << "finish generate texture:" 
	<< cube.texture[0] << std::endl;

	//generate yoz texture
	std::cout << "before generate texture:" 
	<< cube.texture[1] << std::endl;
	four_corners.clear();
	four_corners.push_back(	cv::Point2d(cube.point[3][0],
			       	cube.point[3][1]));    //origin
	four_corners.push_back(	cv::Point2d(cube.point[2][0],
			       	cube.point[2][1]));    //z end
	four_corners.push_back(cv::Point2d(cube.point[5][0], 
				cube.point[5][1]));   //yoz across corner
	four_corners.push_back(cv::Point2d(cube.point[1][0],
			       	cube.point[1][1]));   // y end
	GenerateTextureByDetail(
			four_corners,
		    cube.z_width, 
			cube.y_height, 
			&(cube.texture[1]));
	std::cout << "finish generate texture:" 
	<< cube.texture[1] << std::endl;
}


void Models::GenerateTextureByDetail(
	std::vector<cv::Point2d>& four_corners,
	double width,
	double height,
	GLuint* textureID) {
	cv::Point2f src_point[4], dst_point[4];

	for (unsigned i = 0; i < 4; ++i){
		src_point[i].x = four_corners[i].x;
		src_point[i].y = four_corners[i].y;
	}
	int texture_width =(int)width & ((int)width - 1) == 0 ?
		(int)width :
		pow(2, PowOf2((int)width)) < 64 ? 64 : pow(2, PowOf2((int)width));
	int texture_height = (int)height & ((int)height - 1) == 0 ?
		(int)height :
		pow(2, PowOf2((int) height)) < 64 ? 64 : pow(2, PowOf2((int)height));
	std::cout << "width:" << texture_width << "  height:" << texture_height << std::endl;


	dst_point[0].x = 0;
	dst_point[0].y = static_cast<int> (texture_height);
	dst_point[1].x = static_cast<int> (texture_width);
	dst_point[1].y = static_cast<int> (texture_height);
	dst_point[2].x = static_cast<int> (texture_width);
	dst_point[2].y = 0;
	dst_point[3].x = 0;
	dst_point[3].y = 0;

	cv::Mat src ;
	//src_->copyTo(src);
	foreground_->copyTo(src);

	cv::Mat dst;

	dst.create(cv::Size(texture_width,texture_height), CV_64F);
	cv::Mat warp_mat = cv::Mat(cv::Size(3, 3), CV_32FC1);
	warp_mat = cv::getPerspectiveTransform(src_point, dst_point);
	cv::warpPerspective(src, dst, warp_mat, dst.size());

	//cv::namedWindow("Affine_Transform", 1);
	//cv::imshow("Affine_Transform", dst);

	cv::flip(dst, dst, 0);
	glGenTextures(1, textureID);
	int error = glGetError();
	//std::cout << "2:" << *textureID 
	//	<<" error:"<<error<<":"<<gluErrorString(error)<< std::endl;
	glBindTexture(GL_TEXTURE_2D, *textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
		0,                 // Pyramid level (for mip-mapping) - 0 is the top level
		GL_RGB,            // Internal colour format to convert to
		texture_width,          // Image width 必须是2的n次方
		texture_height,          // Image height 必须是2的n次方
		0,                 // Border width in pixels (can either be 1 or 0)
		GL_BGR_EXT, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		GL_UNSIGNED_BYTE,  // Image data type
		dst.ptr());        // The actual image data itself	
}


void Models::ShowModels(){
	DrawCubes();
}

void Models::model_mouse_callback(int event, int x, int y, int flags, void* param){
	Param<Models>* param_ptr = reinterpret_cast<Param<Models>*> (param);
	int point_index = -1;
	if (event == cv::EVENT_LBUTTONDOWN){
		switch (param_ptr->class_ptr->mode_){
		case SET_X:
			point_index = 0;
			//param_ptr->cube_ptr->point[0][0] = x;
			//param_ptr->cube_ptr->point[0][1] = y;
			std::cout << "choose x end point: (" << x << " , " << y << ")" << std::endl;
			//param_ptr->class_ptr->ShowMarkedPoints(param_ptr->cube_ptr);
			break;
		case SET_Y:
			point_index = 1;
			std::cout << "choose y end point: (" << x << " , " << y << ")" << std::endl;
			break;
		case SET_Z:
			point_index = 2;
			std::cout << "choose z end point: (" << x << " , " << y << ")" << std::endl;
			break;
		case SET_ORIGIN:
			point_index = 3;
			std::cout << "choose origin: (" << x << " , " << y << ")" << std::endl;
			break;
		case SET_XY_ACROSS_CORNER:
			point_index = 4;
			std::cout << "choose xoy plane across point: (" << x << " , " << y << ")" << std::endl;
			break;
		case SET_YZ_ACROSS_CORNER:
			point_index = 5;
			std::cout << "choose yoz plane across point: (" << x << " , " << y << ")" << std::endl;
			break;
		case NONE:
			break;
		}
		if (point_index != -1){
			param_ptr->cube_ptr->point[point_index][0] = x;
			param_ptr->cube_ptr->point[point_index][1] = y;
			param_ptr->class_ptr->ShowMarkedPoints(param_ptr->cube_ptr);
		}
	}

}

void Models::ShowMarkedPoints(Cube* cube){
	if (src_->empty()) {
		std::cout << "ShowImage: image is empty " << std::endl;
		return;
	}
	if (win_name_->empty()) {
		std::cout << "ShowImage: window name is empty" << std::endl;
		return;
	}
	cv::Mat res;
	src_->copyTo(res);

	if (cube->point[0][0] != -1 && cube->point[0][1] != -1) {
		cv::circle(res, cv::Point(cube->point[0][0], cube->point[0][1]), radius, X_END_COLOR, thickness);
	}
	if (cube->point[1][0] != -1 && cube->point[1][1] != -1) {
		cv::circle(res, cv::Point(cube->point[1][0], cube->point[1][1]), radius, Y_END_COLOR, thickness);
	}
	if (cube->point[2][0] != -1 && cube->point[2][1] != -1) {
		cv::circle(res, cv::Point(cube->point[2][0], cube->point[2][1]), radius, Z_END_COLOR, thickness);
	}
	if (cube->point[3][0] != -1 && cube->point[3][1] != -1) {
		cv::circle(res, cv::Point(cube->point[3][0], cube->point[3][1]), radius, ORIGIN_COLOR, thickness);
	}
	if (cube->point[4][0] != -1 && cube->point[4][1] != -1) {
		cv::circle(res, cv::Point(cube->point[4][0], cube->point[4][1]), radius, ACROSS_CORNER_COLOR, thickness);
	}
	if (cube->point[5][0] != -1 && cube->point[5][1] != -1) {
		cv::circle(res, cv::Point(cube->point[5][0], cube->point[5][1]), radius, ACROSS_CORNER_COLOR, thickness);
	}
	//TODO texture marked
	cv::imshow(*win_name_, res);
}

void Models::ShowImage(){
	cv::imshow(*win_name_, *src_);
}
