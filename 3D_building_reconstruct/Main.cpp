#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "common.h"
#include "grabcut.h"
#include "Models.h"
#include "CalibrationImage.h"
#include "RansacForVanishPoint.h"
#include <gl/glut.h>

double g_xrot = 0, g_yrot = 0, g_zrot = 0;
cv::Mat g_src,g_dst,g_gray;
std::vector<PointInfo> g_points;
int g_rows = 0, g_cols = 0;
int ortho[6] = { 0, 0, 0, 0, 0,0 };  //glortho 的六个参数
//int ortho[6] = { -100, 100, -100, 100, -10, 120 };

//*************opengl parameter************
int g_window_width = 800;
int g_window_height = 500;
bool drag;
cv::Point pnow, pmove;
int x_zoom_size, y_zoom_size, z_zoom_size;

GCApplication g_gcapp;
Models g_model_mgr;
cv::Mat g_foreground, g_background;
cv::Mat g_pseudo_inverse_matrix;

void Init(){
	glClearColor(0, 0, 0, 0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
}


void DrawAxis(){
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3i(0, 0, 0);
	glVertex3i(ortho[1], 0, 0);
	glEnd();

	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3i(0, 0, 0);
	glVertex3i(0, ortho[3], 0);
	glEnd();

	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3i(0, 0, 0);
	glVertex3i(0, 0, ortho[5]);
	glEnd();
}



void DisplayFunc() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, ortho[5],  
		0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0);
	glRotatef(g_xrot, 1.0, 0.0, 0.0);
	glRotatef(g_yrot, 0.0, 1.0, 0.0);
	glRotatef(g_zrot, 0.0, 0.0, 1.0);


	DrawAxis();
	g_model_mgr.ShowModels();

/*	glColor3f(0.5, 0.3, 0.4);
	if (start_calibration){
		std::cout << "start to draw point in 3d scene" << std::endl;
	//	cv::destroyWindow(CALIBRATION_IMAGE_WINDOW_NAME);
		//if (g_points.size() > 0) {
		//	glBegin(GL_POINTS);
		//	//		glBegin(GL_LINES);
		//	for (unsigned i = 0; i < g_points.size(); ++i){
		//		glColor3dv(&g_points[i].color[0]);
		//		glVertex3fv(&g_points[i].position[0]);
		//		//			glVertex3f(g_points[i].position[0], g_points[i].position[1], -g_points[i].position[2]);
		//	}
		//	glEnd();
		//}
	}*/
	glutSwapBuffers();
}

void ReshapeFunc(int width, int height) {

	ortho[0] -= pmove.x; ortho[1] -= pmove.x;
	ortho[2] += pmove.y; ortho[3] += pmove.y;
	pmove.x = 0; pmove.y = 0;

	g_window_height = height;
	g_window_width = width;
	
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//视图变幻要在模型变幻之前

	GLdouble znear, zfar;
	znear = 1.0;
	zfar = znear + ortho[5] - ortho[4];
	if (IS_TEST){
		std::cout << "ortho:" << ortho[0] << "," << ortho[1] << "," << ortho[2] << ","
			<< ortho[3] << "," << ortho[4] << "," << ortho[5] <<
			" znear:" << znear << " zfar:" << zfar << std::endl;
	}

	glOrtho(ortho[0], ortho[1], ortho[2], ortho[3], znear, zfar);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void Opengl_help(){
	std::cout <<"Hot key.\n"
		"\tup arrow  - rotate around x axis anticlockwise\n"
		"\tdown arrow  - rotate around x axis clockwise\n"
		"\tleft arrow  - rotate around y axis anticlockwise\n"
		"\tright arrow  - rotate around y axis clockwise\n"

		"\tF1  - zoom in in horizontal\n"
		"\tF2  - zoom out in horizontal\n"
		"\tF3  - zoom in in vertical \n"
		"\tF4  - zoom out in vertical\n"
	//	"\tF5  - move depth in \n"
	//	"\tF6  - move depth out \n"
		"\tclick left mouse button and drag - move sence\n" << std::endl;
}

void SpecialFunc(int key, int x, int y){
	switch (key) {
	case GLUT_KEY_UP:g_xrot += 3.0; break;
	case GLUT_KEY_DOWN:g_xrot -= 3.0; break;
	case GLUT_KEY_LEFT:g_yrot += 3.0; break;
	case GLUT_KEY_RIGHT:g_yrot -= 3.0; break;
	case GLUT_KEY_F1:   
		if (ortho[0] + x_zoom_size > ortho[1] - x_zoom_size){
			std::cout << "x direction already achieve the minimal and can't zoom in" << std::endl;
			return;
		}
		ortho[0] += x_zoom_size; ortho[1] -= x_zoom_size;
		ReshapeFunc(g_window_width, g_window_height);
		break;
	case GLUT_KEY_F2:
		ortho[0] -= x_zoom_size; ortho[1] += x_zoom_size;
		ReshapeFunc(g_window_width, g_window_height);
		break;
	case GLUT_KEY_F3:
		if (ortho[2] + y_zoom_size > ortho[3] - y_zoom_size){
			std::cout << " y direction already achieve the minimal and can't zoom in" << std::endl;
			return;
		}
		ortho[2] += y_zoom_size; 
		ortho[3] -= y_zoom_size; 
		ReshapeFunc(g_window_width, g_window_height); 
		break;
	case GLUT_KEY_F4:
		ortho[2] -= y_zoom_size; 
		ortho[3] += y_zoom_size;
		ReshapeFunc(g_window_width, g_window_height); 
		break;
/*	case GLUT_KEY_F5:
		znear += z_zoom_size;
		zfar += z_zoom_size;
		ReshapeFunc(g_window_width, g_window_height); 
		break;
	case GLUT_KEY_F6:
		znear -=  z_zoom_size;
		zfar -= z_zoom_size;		
		ReshapeFunc(g_window_width, g_window_height); 
		break;  */
	}
	glutPostRedisplay();
}

void OpenGLMouseCallback(int button, int state, int x, int y){

	drag = ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN));
	if (drag){
		pnow.x = x; pnow.y = y;
	}
	glutPostRedisplay();
}

void OpenGLMotionCallback(int x, int y){
	if (drag){
		pmove.x = x - pnow.x;
		pmove.y = y - pnow.y;
		pnow.x = x; pnow.y = y;
		ReshapeFunc(g_window_width, g_window_height);
	}
}


void mouse_callback(int event, int x, int y, int flag, void* param){
	CalibrationImage* ci_ptr = reinterpret_cast<CalibrationImage*> (param);
	
	if (event == CV_EVENT_LBUTTONDOWN){
		if (!ci_ptr->LinePointAllSet(X)){
			ci_ptr->SetPoint(X, x, y);
			ci_ptr->ShowSrc();
			return;
		}
		else if (!ci_ptr->LinePointAllSet(Y)){
			ci_ptr->SetPoint(Y, x, y);
			ci_ptr->ShowSrc();			
			return;
		}
		else if (!ci_ptr->LinePointAllSet(Z)){
			ci_ptr->SetPoint(Z, x, y);
			ci_ptr->ShowSrc();
			return;
		}
		ci_ptr->ChooseWorldOrigin(x, y);
	}

}

static void grabcut_on_mouse(int event, int x, int y, int flags, void* param)
{
	g_gcapp.mouseClick(event, x, y, flags, param);
}

bool vanishing_point_ok(cv::Point2d vanishing_point[],int size){
	for (int i = 0; i < size; ++i){
		if (vanishing_point[i].x == 0 && vanishing_point[i].y == 0){
			std::cout << "\nNOTICE:vanishing point " << i + 1 << " is not set successfully,please set again\n" << std::endl;
			return false;
		}
	}
	return true;
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		g_gcapp.help();
		return 1;
	}
	std::string filename = argv[1];
	if (filename.empty()) {
		std::cout << "\nDurn, couldn't read in " << argv[1] << std::endl;
		return 1;
	}
	g_src = cv::imread(filename, 1);
	if (g_src.empty()) {
		std::cout << "\n Durn, couldn't read image filename " << filename << std::endl;
		return 1;
	}


	g_gcapp.help();

	std::string win_name = GRABCUT_SRC_WINDOW_NAME;
	cv::namedWindow(win_name, cv::WINDOW_AUTOSIZE);
	cv::setMouseCallback(win_name, grabcut_on_mouse, 0);
	g_gcapp.setImageAndWinName(g_src,win_name);
	g_gcapp.showImage();  

	while(1) {
		int c = cv::waitKey(0);
		switch ((char)c) {
		case '\x1b':
			std::cout << "Finsh Grabcut ..." << std::endl;		
			g_foreground = g_gcapp.returnForeground();
			g_background = g_src - g_foreground;
			cv::destroyWindow(GRABCUT_SRC_WINDOW_NAME);
			//goto auto_calc_pseudo_matrix;
			goto choose;
		case 'r':
			std::cout << std::endl;
			g_gcapp.reset();
			g_foreground.setTo(cv::Scalar(0));
			g_background.setTo(cv::Scalar(0));
			g_gcapp.showImage();
			break;
		case 'n':
			int iterCount = g_gcapp.getIterCount();
			std::cout << "<" << iterCount << "... ";
			int newIterCount = g_gcapp.nextIter();
			if (newIterCount > iterCount) {
				g_gcapp.showImage();
				std::cout << iterCount << ">" << std::endl;
			} else {
				std::cout << "rect must be determined>" << std::endl;
			}
			break;
		}
	}
choose: {
	std::cout << "\t1.auto vanishing point calc\n"
		"\t2.user interaction to calc vanishing point\n"
		"please choose:";
	while (1) {
		char input;
		std::cin >> input;
		switch (input) {
		case '1':
			goto auto_calc_pseudo_matrix;
		case '2':
			goto calc_pseudo_matrix;
		}
	}
}
auto_calc_pseudo_matrix: {
	std::string calibratin_win_name = CALIBRATION_IMAGE_WINDOW_NAME;
	cv::namedWindow(calibratin_win_name, cv::WINDOW_AUTOSIZE);
	cv::moveWindow(calibratin_win_name, 500, 10);

	RansacForVanishPoint test(calibratin_win_name, g_src);
	cv::Point2d vanishing_point[3];
	test.CalcVanishingPoints(vanishing_point, 3);
	//std::cout << "Press n/N to continue..." << std::endl;
	cv::waitKey(0);
	
	while (1) {
		char input;
		std::cin >> input;
		switch (input) {
		case 'n':
		case 'N':{
			if (!vanishing_point_ok(vanishing_point, 3)){
			//	break;
				goto choose;
			}
			CalibrationImage ci = CalibrationImage(filename, g_foreground);
			ci.SetVanishingPoint(vanishing_point);
			if (!ci.SetCalibrationMatrix()) {
				goto choose;
			}
			g_pseudo_inverse_matrix = ci.GetPseudoInverseMatrix();

			test.DestroyWindow();
			goto choose_model;
		}
		}
	}
}
calc_pseudo_matrix:{
	cv::destroyAllWindows();
	CalibrationImage ci = CalibrationImage(filename, g_src);
	ci.Help();
	cv::namedWindow(CALIBRATION_IMAGE_WINDOW_NAME, cv::WINDOW_AUTOSIZE);
	CalibrationImage *ptr = &ci;
	cv::setMouseCallback(CALIBRATION_IMAGE_WINDOW_NAME, mouse_callback, reinterpret_cast<void*>(ptr));
	ci.ShowSrc();

	while (1) {
		int c = cv::waitKey(0);
		switch ((char)c) {
		case 'r':
		case 'R':
			std::cout << std::endl;
			ci.Reset();
			ci.ShowSrc();
			//goto calc_pseudo_matrix;
			break;
		case 'n':
		case 'N':
			if (ci.LinePointAllSet(X) && ci.LinePointAllSet(Y) && ci.LinePointAllSet(Z)) {
				ci.SetVanishingPoint();
				ci.SetCalibrationMatrix();
				//	ci.ChangePointTo3dinOpenGL(g_points, ortho, &g_rows, &g_cols);
			}
			break;
		case 'g':
		case 'G':
			if (ci.IsFinish()){
				cv::destroyWindow(CALIBRATION_IMAGE_WINDOW_NAME);
				g_pseudo_inverse_matrix = ci.GetPseudoInverseMatrix();
				goto choose_model;
			}
		}
	}
}
	
choose_model:{
	//IMPORTANT:生成模型的时候会生成纹理，必须在之前初始化opengl
	glutInit(&argc, argv);  //glut 初始化
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(g_window_width, g_window_height);
	glutInitWindowPosition(100, 20);
	glutCreateWindow(GL_MAIN_WINDOW_NAME);
	Init();
	//生成模型时需要先生成glut窗口
	//生成模型前必须先求出伪逆矩阵
	std::string model_win_name = CHOOSE_MODEL_WINDOW_NAME;
	cv::namedWindow(model_win_name, cv::WINDOW_AUTOSIZE);
	cv::imshow(model_win_name, g_src);
	if (g_model_mgr.SetImageWindowAndMatrix(g_src, g_foreground,model_win_name, g_pseudo_inverse_matrix)){
		//g_model_mgr.ShowImage();
		bool try_again = false;
		char input;
		do {
			std::cout << "\n\ts - show models in opengl"
				<< "\t1-add cube " << std::endl
				<< "please choose:";
			std::cin >> input;
			switch (input){
			case '1':
				g_model_mgr.help();
				g_model_mgr.AddCube(ortho);

				x_zoom_size = (ortho[1] - ortho[0]) / 20;
				y_zoom_size = (ortho[3] - ortho[2]) / 20;
				z_zoom_size = (ortho[5] - ortho[4]) / 20;
				

				try_again = true;
				break;
			case 's':
			case 'S':
				cv::destroyWindow(model_win_name);
				try_again = false;
				goto display_in_opengl;
			default:
				std::cout << "wrong input.input again" << std::endl;
				try_again = true;
			}
		} while (try_again);
	}
}
display_in_opengl: {
   Opengl_help();

	glutDisplayFunc(DisplayFunc);
	glutReshapeFunc(ReshapeFunc);
	glutSpecialFunc(SpecialFunc);
	glutMouseFunc(OpenGLMouseCallback);
	glutMotionFunc(OpenGLMotionCallback);
	glutMainLoop();
}
	return 0; 
}
