#pragma once
#include "glew.h"
#include <functional>


//����buffer ����vbo��iboʱ�����Ե��ô˺���
GLuint CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data = nullptr);
char* LoadFileContent(const char* path);
GLuint CompileShader(GLenum shaderType, const char* shaderPath);
//������ɫ������
GLuint CreateGPUProgram(const char* vsShaderPath, const char* fsShaderPath);
// �Ӹ�����Ŀ¼�м��������ļ�
GLuint CreateTextureFromFile(const char* imagePath);

GLuint CreateVAOWithVBOSettings(std::function<void()>recordSetting);

GLuint CreateFrameBufferObject(GLuint& colorBuffer, GLuint& depthBuffer, int width, int height);

void CheckGLError(const char* file, int line);
#define GL_CALL(x) do{ x;CheckGLError(__FILE__,__LINE__);}while (0)