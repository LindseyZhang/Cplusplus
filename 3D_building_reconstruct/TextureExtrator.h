#pragma once
#include "Models.h"
#include "common.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <gl/glut.h>

enum TextureType {
	FRONT_TEXTURE,
	SIDE_TEXTURE,
};

class TextureExtrator
{
public:
	TextureExtrator(const std::string* win_name_);
	~TextureExtrator();
	void Help();
	void ChooseTexture(TextureType type,Cube* cube);
private:
	static void Texture_mouse_callback(int event, int x, int y, int flags, void* param);

	const std::string* win_name_;

};

