#include "TextureExtrator.h"
#include "common.h"
#include <iostream>

TextureExtrator::TextureExtrator(const std::string* win_name)
{
	win_name_ = win_name;
}


TextureExtrator::~TextureExtrator()
{
}
//texture 须在进来之前声明好,大小为2的整数倍 
void TextureExtrator::ChooseTexture(TextureType type, Cube* cube){
	Param<TextureExtrator> param;
	param.class_ptr = this;
	param.cube_ptr = cube;
	cv::setMouseCallback(*win_name_, Texture_mouse_callback, this);
}
void TextureExtrator::Texture_mouse_callback(int event, int x, int y, int flags, void* param){
	Param<TextureExtrator>* param_ptr = reinterpret_cast<Param<TextureExtrator>*> (param);
	std::cout << " texture_mouse_callback works fine" << std::endl;
}
