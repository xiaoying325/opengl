#pragma once
#include "glew.h"

GLuint CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data = nullptr);
char* LoadFileContent(const char* path);
GLuint CreateTextureFromFile(const char* imagePath);
void SaveImage(const char* imagePath, unsigned char* imgData, int width, int height);