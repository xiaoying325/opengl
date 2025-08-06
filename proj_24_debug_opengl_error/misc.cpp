
#include <stdio.h>
#include "glew.h"
#include<string.h>



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

//GLuint CreateTextureFromFile(const char* imagePath)
//{
//	unsigned char* imgData = (unsigned char*)LoadFileContent(imagePath);
//
//	// 1.����u����BMP�ļ�����У��
//	if (*((unsigned short*)imgData) != 0x4D42) { // 0x4D42 ��ʾ��ASCII�� ��B M �պ�BMP�ļ��ĸ�ʽ�����ؼ�ͷ�����ֽھ���������
//		//������Ƕ�ȡ��������������ļ���ͷ��ʶ�����ֲ���bmg���ؼ�
//		printf("cannot decode %s\n", imagePath);
//		delete imgData;
//		return 0;
//	}
//
//	// 2.��ȡͼ������
//	// BMP�ļ���ʽ�涨��
//	//11~14 �ֽڣ�����������ʼλ�ã�Offset��
//	//18~21 �ֽڣ�ͼ���ȣ�int��
//	//22~25 �ֽڣ�ͼ��߶ȣ�int��
//	int pixelDataOffset = *((int*)(imgData + 10)); // ��ImgData��ʼ��ַ + 10���ֽڣ� �ǵ�11���ֽڿ�ʼ����ȡint ��4���ֽ����ݣ� 11 12 13 14  ������bmg�������ݵ���ʼƫ��λ��
//	int width = *((int*)(imgData + 18));// 18 19 20 21  ���
//	int height = *((int*)(imgData + 22));//  22 23 24 25 �߶�
//
//	unsigned char* pixelData = (imgData + pixelDataOffset); //�������ݵ�ַ��ʼƫ��������
//
//	///Byte 0	Byte 1	Byte 2	Byte 3	Byte 4	Byte 5	Byte 6	Byte 7	Byte 8	Byte 9	Byte 10	Byte 11	Padding Bytes...
//	//  B	       G	     R	      B	      G	      R	     B	      G       R       B       G      R    ......
//	for (int i = 0; i < width * height*3; i += 3) {
//		// Opengl��Ҫ����RGB���֣�����������ļ���BGR�ģ������ǲ���Ҫתһ�£��Բ���
//		unsigned char temp = pixelData[i + 2]; //R
//		pixelData[i + 2] = pixelData[i];
//		pixelData[i] = temp;
//	}
//
//	// 3.�����Ѿ���������pixelData�����ˣ�����������Ҫ��ʲô��
//	// ?����Opengl���������
//	GLuint texture; //�������������
//	glGenTextures(1, &texture);//����opengl�ֿ����Ա����Ҫ����1�Ż�����������������������ݵ�
//	glBindTexture(GL_TEXTURE_2D, texture);//����opengl�ֿ����Ա����һ���ҩ�����ݵ����������
//
//	//ST VU 
//	// ���˷�ʽ warp mode
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);// V ��ֱ�����ϳ���[0,1]֮��������ô����GL_CLAMP_TO_EDGE��ʾ������Ե��ɫ�����ظ����߾���
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// V ��ֱ�����ϳ���[0,1]֮��������ô����GL_CLAMP_TO_EDGE��ʾ������Ե��ɫ�����ظ����߾���
//
//	// ����ģʽ
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //���� ,GL_NEAREST ����ڲ���
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	//û��minMapy
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	delete imgData;
//	return texture;
//
//}