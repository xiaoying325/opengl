#include "misc.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <functional>





GLuint CreateFrameBufferObject(GLuint& colorBuffer, GLuint& depthBuffer, int width, int height) {
	//1.����fbo
	GLuint fbo;
	// ����⣺�񹤳����㷢��һ��Ψһ��ţ�ID�������Ժ���������Ŵ����Framebuffer��1 ����˵��Ҫ�����������1�����
	glGenFramebuffers(1, &fbo);// ����һ��framebuffer����fbo������id   ֻ������һ��id��Ȼ��󶨣������� 1 ����һ�����ڶ�������ָ�룬����opengl�����ɵ�idд���������������д��fbo��
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);//�󶨸�fbo���������������������


	// ������ɫ��������
	glGenTextures(1, &colorBuffer);               // 3. ����һ������ID��������colorBuffer������
	glBindTexture(GL_TEXTURE_2D, colorBuffer);    // 4. �󶨸��������
	// 5. ����������������Թ��ˣ��������곬����Χʱ��Ե���죨CLAMP_TO_EDGE��
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// 6. ���������ڴ棬��ʽΪRGBA8��8λRGBA��������ɲ�������������Ϊ�գ�nullptr��
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);              // 7. ���������ֹ�����
	// 8. ���������ӵ���ǰ�󶨵�Framebuffer����ɫ����0��
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);


	// ������Ȼ���������
	glGenTextures(1, &depthBuffer);                // 9. ����һ������ID��������depthBuffer������
	glBindTexture(GL_TEXTURE_2D, depthBuffer);     // 10. �󶨸��������
	// 11. �������������ͬ��ɫ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// 12. ������������ڴ棬��ʽΪ24λ��ȣ�����ɲ�������������Ϊ��
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);               // 13. �������
	// 14. ������������ӵ���ǰFramebuffer����ȸ�����
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);


	// �����framebuffer�������ԣ�ȷ�����и�������������
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		// 16. �������������ӡ������Ϣ�����ͷ����������Դ������0��ʾʧ��
		printf("create framebuffer object fail: 0x%x\n", status);
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &colorBuffer);
		glDeleteTextures(1, &depthBuffer);
		fbo = 0;
	}

	// 17. ���Framebuffer���л���Ĭ��Framebuffer������ϵͳ�ṩ�ģ�
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 18. ���ش�����Framebuffer����ID����ʧ��ʱ0��
	return fbo;


}


GLuint CreateVAOWithVBOSettings(std::function<void()>recordSetting)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	if (recordSetting) {
		recordSetting(); //�Ѳ�����Ϊ����¼��������¼��vao�У� �Ͳ���Ҫÿ�����²�����Щ�ظ��Ĳ�����
	}
	
	glBindVertexArray(0);
	return vao;
}


GLuint CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data /* = nullptr */)
{
	GLuint object;
	glGenBuffers(1, &object);
	glBindBuffer(bufferType, object);
	glBufferData(bufferType, size, data, usage);
	glBindBuffer(bufferType, 0);
	return object;
}


char* LoadFileContent(const char* path)
{
	FILE* pFile = fopen(path, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		int nLen = ftell(pFile);
		char* buffer = nullptr;
		if (nLen != 0)
		{
			buffer = new char[nLen + 1];
			rewind(pFile);
			fread(buffer, nLen, 1, pFile);
			buffer[nLen] = '\0';
		}
		else
		{
			printf("load file fail %s content len is 0\n", path);
		}
		fclose(pFile);
		return buffer;
	}
	else
	{
		printf("open file %s fail\n", path);
	}
	fclose(pFile);
	return nullptr;
}


GLuint CompileShader(GLenum shaderType, const char* shaderPath)
{
	GLuint shader = glCreateShader(shaderType);
	if (shader == 0)
	{
		printf("glCreateShader fail\n");
		return 0;
	}
	const char* shaderCode = LoadFileContent(shaderPath);
	if (shaderCode == nullptr)
	{
		printf("load shader code from file : %s fail\n", shaderPath);
		glDeleteShader(shader);
		return 0;
	}
	glShaderSource(shader, 1, &shaderCode, nullptr);
	glCompileShader(shader);
	GLint compileResult = GL_TRUE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
	if (compileResult == GL_FALSE)
	{
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;
		glGetShaderInfoLog(shader, 1024, &logLen, szLog);
		printf("Compile Shader fail error log : %s \nshader code :\n%s\n", szLog, shaderCode);
		glDeleteShader(shader);
		shader = 0;
	}
	delete shaderCode;
	return shader;
}


GLuint CreateGPUProgram(const char* vsShaderPath, const char* fsShaderPath)
{
	GLuint vsShader = CompileShader(GL_VERTEX_SHADER, vsShaderPath);
	GLuint fsShader = CompileShader(GL_FRAGMENT_SHADER, fsShaderPath);
	GLuint program = glCreateProgram();
	glAttachShader(program, vsShader);
	glAttachShader(program, fsShader);
	glLinkProgram(program);
	glDetachShader(program, vsShader);
	glDetachShader(program, fsShader);
	glDeleteShader(vsShader);
	glDeleteShader(fsShader);
	GLint linkResult = GL_TRUE;
	glGetProgramiv(program, GL_LINK_STATUS, &linkResult);
	if (linkResult == GL_FALSE)
	{
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;
		glGetProgramInfoLog(program, 1024, &logLen, szLog);
		printf("link program fail error log : %s \n vs : %s\n fs : %s\n", szLog, vsShaderPath, fsShaderPath);
		glDeleteProgram(program);
		program = 0;
	}
	return program;
}

static unsigned char* DecodeBMPData(unsigned char* imgData, int& width, int& height) {
	// 2.��ȡͼ������
	// BMP�ļ���ʽ�涨��
	//11~14 �ֽڣ�����������ʼλ�ã�Offset��
	//18~21 �ֽڣ�ͼ���ȣ�int��
	//22~25 �ֽڣ�ͼ��߶ȣ�int��
	int pixelDataOffset = *((int*)(imgData + 10)); // ��ImgData��ʼ��ַ + 10���ֽڣ� �ǵ�11���ֽڿ�ʼ����ȡint ��4���ֽ����ݣ� 11 12 13 14  ������bmg�������ݵ���ʼƫ��λ��
	width = *((int*)(imgData + 18));// 18 19 20 21  ���
	height = *((int*)(imgData + 22));//  22 23 24 25 �߶�
	unsigned char* pixelData = (imgData + pixelDataOffset); //�������ݵ�ַ��ʼƫ��������

	///Byte 0	Byte 1	Byte 2	Byte 3	Byte 4	Byte 5	Byte 6	Byte 7	Byte 8	Byte 9	Byte 10	Byte 11	Padding Bytes...
	//  B	       G	     R	      B	      G	      R	     B	      G       R       B       G      R    ......
	for (int i = 0; i < width * height*3; i += 3) {
		// Opengl��Ҫ����RGB���֣�����������ļ���BGR�ģ������ǲ���Ҫתһ�£��Բ���
		unsigned char temp = pixelData[i + 2]; //R
		pixelData[i + 2] = pixelData[i];
		pixelData[i] = temp;
	}

	return pixelData;
}


static unsigned char* DecodeDXT1Data(unsigned char* imgData, int& width, int& height, int& pixelDataSize) {
	//����Dxt1 ����������
	// �ṹ�ƺ���εģ�
	const unsigned long FORMAT_DXT1 = 0x31545844; // 0x31  1 0x54 T 0x58 X 0x44 D 
	width = *(unsigned long*)(imgData + 4 * 4);   // offset 16
	height = *(unsigned long*)(imgData + 4 * 3);   // offset 12
	pixelDataSize = *(unsigned long*)(imgData + 4 * 5); // offset 20
	unsigned long fourCC = *(unsigned long*)(imgData + 4 * 21); // offset 84

	if (fourCC != FORMAT_DXT1)
	{
		printf("Not a DXT1 texture\n");
		return nullptr;
	}

	// DDS header is 128 bytes
	unsigned char* pixelData = new unsigned char[pixelDataSize];
	memcpy(pixelData, imgData + 128, pixelDataSize);
	return pixelData;

}


GLuint CreateTextureFromFile(const char* imagePath) {
	unsigned char* imgData = (unsigned char*)LoadFileContent(imagePath);
	int width = 0;
	int height = 0;
	unsigned char* pixelData = nullptr;

	int pixelDataSize = 0;

	GLenum srcFormat = GL_RGB; //��ʼ��Ĭ����������ΪRGB
	if (*((unsigned short*)imgData) == 0x4D42) {
		// ��ʾ�鿴ǰ�����ֽ� 
		pixelData = DecodeBMPData(imgData, width, height);
	}
	else if (memcmp(imgData, "DDS ", 4)==0) {
		//���۵��ڼ��صľ���dxt1 ��ʽ������

		pixelData = DecodeDXT1Data(imgData, width, height, pixelDataSize);
		srcFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; //����������opnegl�оͱ�ʾ��ʹ����dxt1ѹ��������
	}
	if (pixelData == nullptr) {
		delete pixelData;
		return 0;
	}

		// 3.�����Ѿ���������pixelData�����ˣ�����������Ҫ��ʲô��
	// ?����Opengl���������
	GLuint texture; //�������������
	glGenTextures(1, &texture);//����opengl�ֿ����Ա����Ҫ����1�Ż�����������������������ݵ�
	glBindTexture(GL_TEXTURE_2D, texture);//����opengl�ֿ����Ա����һ���ҩ�����ݵ����������

	//ST VU 
	// ���˷�ʽ warp mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);// V ��ֱ�����ϳ���[0,1]֮��������ô����GL_CLAMP_TO_EDGE��ʾ������Ե��ɫ�����ظ����߾���
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// V ��ֱ�����ϳ���[0,1]֮��������ô����GL_CLAMP_TO_EDGE��ʾ������Ե��ɫ�����ظ����߾���

	// ����ģʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //���� ,GL_NEAREST ����ڲ���
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//û��minMapy

	if (srcFormat==GL_RGB) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	}
	else if (srcFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) {
		//������Ҫ���� ר�������ϴ�ѹ�������API��glCompressedTexImage2D
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, width, height, 0, pixelDataSize, pixelData);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	delete imgData;
	return texture;


}
void CheckGLError(const char* file, int line)
{

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		switch (error)
		{
		case  GL_INVALID_ENUM:
			printf("GL Error GL_INVALID_ENUM %s : %d\n", file, line);
			break;
		case  GL_INVALID_VALUE:
			printf("GL Error GL_INVALID_VALUE %s : %d\n", file, line);
			break;
		case  GL_INVALID_OPERATION:
			printf("GL Error GL_INVALID_OPERATION %s : %d\n", file, line);
			break;
		case  GL_STACK_OVERFLOW:
			printf("GL Error GL_STACK_OVERFLOW %s : %d\n", file, line);
			break;
		case  GL_STACK_UNDERFLOW:
			printf("GL Error GL_STACK_UNDERFLOW %s : %d\n", file, line);
			break;
		case  GL_OUT_OF_MEMORY:
			printf("GL Error GL_OUT_OF_MEMORY %s : %d\n", file, line);
			break;
		default:
			printf("GL Error 0x%x %s : %d\n", error, file, line);
			break;
		}
	}
}
