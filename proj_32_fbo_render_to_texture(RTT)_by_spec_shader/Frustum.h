#pragma once

#include "glew.h"
#include "Glm/glm.hpp"
#include "Glm/ext.hpp"


class Frustum
{
public:
	GLuint mVBO, mIBO, mProgram;
	GLint mPLocation, mVLocation, mMLocation, mPosLocation;

public:
	Frustum();
	void InitProgram();
	void InitPrespective(float fov, float aspect, float zNear, float zFar);
	void InitOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
	void Draw(float* M, float* V, float* P);
};

