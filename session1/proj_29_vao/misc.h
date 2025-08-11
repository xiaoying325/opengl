#pragma once
#include "glew.h"
#include <functional>


//创建buffer 创建vbo和ibo时都可以调用此函数
GLuint CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data = nullptr);
char* LoadFileContent(const char* path);
GLuint CompileShader(GLenum shaderType, const char* shaderPath);
//创建着色器程序
GLuint CreateGPUProgram(const char* vsShaderPath, const char* fsShaderPath);
// 从给定的目录中加载纹理文件
GLuint CreateTextureFromFile(const char* imagePath);

GLuint CreateVAOWithVBOSettings(std::function<void()>recordSetting);

void CheckGLError(const char* file, int line);
#define GL_CALL(x) do{ x;CheckGLError(__FILE__,__LINE__);}while (0)