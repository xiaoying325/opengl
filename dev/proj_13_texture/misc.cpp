
#include <stdio.h>
#include "glew.h"




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



GLuint CreateTextureFromFile(const char* imagePath)
{
	unsigned char* imgData = (unsigned char*)LoadFileContent(imagePath);

	// 1.对是u不是BMP文件进行校验
	if (*((unsigned short*)imgData) != 0x4D42) { // 0x4D42 表示是ASCII码 的B M 刚好BMP文件的格式的我呢见头两个字节就是这玩意
		//如果我们读取这个解析出来的文件的头标识，发现不是bmg我呢见
		printf("cannot decode %s\n", imagePath);
		delete imgData;
		return 0;
	}

	// 2.读取图像数据
	// BMP文件格式规定：
	//11~14 字节：像素数据起始位置（Offset）
	//18~21 字节：图像宽度（int）
	//22~25 字节：图像高度（int）
	int pixelDataOffset = *((int*)(imgData + 10)); // 从ImgData起始地址 + 10个字节， 那第11个字节开始，读取int （4个字节数据） 11 12 13 14  独到的bmg像素数据的起始偏移位置
	int width = *((int*)(imgData + 18));// 18 19 20 21  宽度
	int height = *((int*)(imgData + 22));//  22 23 24 25 高度

	unsigned char* pixelData = (imgData + pixelDataOffset); //像素数据地址起始偏移那里了

	///Byte 0	Byte 1	Byte 2	Byte 3	Byte 4	Byte 5	Byte 6	Byte 7	Byte 8	Byte 9	Byte 10	Byte 11	Padding Bytes...
	//  B	       G	     R	      B	      G	      R	     B	      G       R       B       G      R    ......
	for (int i = 0; i < width * height*3; i += 3) {
		// Opengl他要的是RGB这种，但是你这个文件是BGR的，我们是不是要转一下？对不对
		unsigned char temp = pixelData[i + 2]; //R
		pixelData[i + 2] = pixelData[i];
		pixelData[i] = temp;
	}

	// 3.数据已经被解析到pixelData中来了，接下里我们要做什么呢
	// ?创建Opengl的纹理对象
	GLuint texture; //创建纹理缓冲对象
	glGenTextures(1, &texture);//告诉opengl仓库管理员，我要操作1号缓冲对象，是用来处理纹理数据的
	glBindTexture(GL_TEXTURE_2D, texture);//告诉opengl仓库管理员，我一会儿药房数据到这个格子中

	//ST VU 
	// 过滤方式 warp mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);// V 垂直方向上超出[0,1]之后我们怎么处理，GL_CLAMP_TO_EDGE表示采样边缘杨色，不重复或者镜像
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// V 垂直方向上超出[0,1]之后我们怎么处理，GL_CLAMP_TO_EDGE表示采样边缘杨色，不重复或者镜像

	// 缩放模式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //线性 ,GL_NEAREST 最近邻采样
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//没有minMap

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	glBindTexture(GL_TEXTURE_2D, 0);
	delete imgData;
	return texture;









}