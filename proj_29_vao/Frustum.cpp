#include "Frustum.h"
#include "misc.h"


Frustum::Frustum() {

}

void Frustum::InitProgram() {
	mProgram = CreateGPUProgram("res/shader/WhiteColor.vs", "res/shader/WhiteColor.fs");
	mPosLocation = glGetAttribLocation(mProgram, "pos");

	mMLocation = glGetUniformLocation(mProgram, "M");
	mVLocation = glGetUniformLocation(mProgram, "V");
	mPLocation = glGetUniformLocation(mProgram, "P");
}

// fov aspect zNear zfar 这几个参数肯定是要传递进来的，不然你怎么计算那8个点的坐标呢？
void Frustum::InitPrespective(float fov, float aspect, float zNear, float zFar) {
	//near clip plane
	float halfFOV = 22.5f;
	float randianHalfFOV = 3.14f * halfFOV / 180.0f;
	float tanHalfFOV = sinf(randianHalfFOV) / cosf(randianHalfFOV);
	float yNear = tanHalfFOV * zNear;
	float xNear = yNear * aspect;

	//far clip plane
	float yFar = tanHalfFOV * zFar;
	float xFar = yFar * aspect;

	float vertexes[24] =
	{
		-xNear,-yNear,-zNear, // x  y z 
		xNear,-yNear,-zNear,
		xNear,yNear,-zNear,
		-xNear,yNear,-zNear,

		-xFar,-yFar,-zFar,
		xFar,-yFar,-zFar,
		xFar,yFar,-zFar,
		-xFar,yFar,-zFar,
	};
	mVBO = CreateBufferObject(GL_ARRAY_BUFFER, sizeof(float) * 24, GL_STATIC_DRAW, vertexes);
	unsigned int indexes[] = {
		0,1,1,2,2,3,3,0,//near plane
		4,5,5,6,6,7,7,4,//far plane
		0,4,3,7,2,6,1,5
	};
	mIBO = CreateBufferObject(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 24, GL_STATIC_DRAW, indexes);
}

void Frustum::InitOrtho(float left, float right, float bottom, float top, float zNear, float zFar) {
	float vertexes[24] =
	{
		left,bottom,-zNear,
		right,bottom,-zNear,
		right,top,-zNear,
		left,top,-zNear,

		left,bottom,-zFar,
		right,bottom,-zFar,
		right,top,-zFar,
		left,top,-zFar,
	};
	mVBO = CreateBufferObject(GL_ARRAY_BUFFER, sizeof(float) * 24, GL_STATIC_DRAW, vertexes);
	unsigned int indexes[] = {
		0,1,1,2,2,3,3,0,//near plane
		4,5,5,6,6,7,7,4,//far plane
		0,4,3,7,2,6,1,5
	};
	mIBO = CreateBufferObject(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 24, GL_STATIC_DRAW, indexes);
}

//把视锥体线框绘制出来
void Frustum::Draw(float* M, float* V, float* P) {
	glUseProgram(mProgram);
	glUniformMatrix4fv(mMLocation, 1, GL_FALSE, M);
	glUniformMatrix4fv(mVLocation, 1, GL_FALSE, V);
	glUniformMatrix4fv(mPLocation, 1, GL_FALSE, P);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glEnableVertexAttribArray(mPosLocation);
	glVertexAttribPointer(mPosLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);// GL_LINES 使用划线模式
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}