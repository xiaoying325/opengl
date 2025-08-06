#pragma once
#include "glew.h"

GLuint CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data = nullptr);
char* LoadFileContent(const char* path);
GLuint CreateTextureFromFile(const char* imagePath);

void CheckGLError(const char* file, int line);
#define GL_CALL(x) do{ x;CheckGLError(__FILE__,__LINE__);}while (0)