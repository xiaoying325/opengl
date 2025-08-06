
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
	// 2.读取图像数据
	// BMP文件格式规定：
	//11~14 字节：像素数据起始位置（Offset）
	//18~21 字节：图像宽度（int）
	//22~25 字节：图像高度（int）
	int pixelDataOffset = *((int*)(imgData + 10)); // 从ImgData起始地址 + 10个字节， 那第11个字节开始，读取int （4个字节数据） 11 12 13 14  独到的bmg像素数据的起始偏移位置
	width = *((int*)(imgData + 18));// 18 19 20 21  宽度
	height = *((int*)(imgData + 22));//  22 23 24 25 高度
	unsigned char* pixelData = (imgData + pixelDataOffset); //像素数据地址起始偏移那里了

	///Byte 0	Byte 1	Byte 2	Byte 3	Byte 4	Byte 5	Byte 6	Byte 7	Byte 8	Byte 9	Byte 10	Byte 11	Padding Bytes...
	//  B	       G	     R	      B	      G	      R	     B	      G       R       B       G      R    ......
	for (int i = 0; i < width * height*3; i += 3) {
		// Opengl他要的是RGB这种，但是你这个文件是BGR的，我们是不是要转一下？对不对
		unsigned char temp = pixelData[i + 2]; //R
		pixelData[i + 2] = pixelData[i];
		pixelData[i] = temp;
	}

	return pixelData;
}


static unsigned char* DecodeDXT1Data(unsigned char* imgData, int& width, int& height, int& pixelDataSize) {
	//解码Dxt1 纹理、？？？
	// 结构似乎如何的？
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

	GLenum srcFormat = GL_RGB; //初始化默认纹理类型为RGB
	if (*((unsigned short*)imgData) == 0x4D42) {
		// 表示查看前两个字节 
		pixelData = DecodeBMPData(imgData, width, height);
	}
	else if (memcmp(imgData, "DDS ", 4)==0) {
		//标售档期加载的就是dxt1 格式的纹理

		pixelData = DecodeDXT1Data(imgData, width, height, pixelDataSize);
		srcFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; //这种类型在opnegl中就表示是使用了dxt1压缩的纹理
	}
	if (pixelData == nullptr) {
		delete pixelData;
		return 0;
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

	//没有minMapy

	if (srcFormat==GL_RGB) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	}
	else if (srcFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) {
		//我们需要调用 专门用来上传压缩纹理的API，glCompressedTexImage2D
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
//	// 1.对是u不是BMP文件进行校验
//	if (*((unsigned short*)imgData) != 0x4D42) { // 0x4D42 表示是ASCII码 的B M 刚好BMP文件的格式的我呢见头两个字节就是这玩意
//		//如果我们读取这个解析出来的文件的头标识，发现不是bmg我呢见
//		printf("cannot decode %s\n", imagePath);
//		delete imgData;
//		return 0;
//	}
//
//	// 2.读取图像数据
//	// BMP文件格式规定：
//	//11~14 字节：像素数据起始位置（Offset）
//	//18~21 字节：图像宽度（int）
//	//22~25 字节：图像高度（int）
//	int pixelDataOffset = *((int*)(imgData + 10)); // 从ImgData起始地址 + 10个字节， 那第11个字节开始，读取int （4个字节数据） 11 12 13 14  独到的bmg像素数据的起始偏移位置
//	int width = *((int*)(imgData + 18));// 18 19 20 21  宽度
//	int height = *((int*)(imgData + 22));//  22 23 24 25 高度
//
//	unsigned char* pixelData = (imgData + pixelDataOffset); //像素数据地址起始偏移那里了
//
//	///Byte 0	Byte 1	Byte 2	Byte 3	Byte 4	Byte 5	Byte 6	Byte 7	Byte 8	Byte 9	Byte 10	Byte 11	Padding Bytes...
//	//  B	       G	     R	      B	      G	      R	     B	      G       R       B       G      R    ......
//	for (int i = 0; i < width * height*3; i += 3) {
//		// Opengl他要的是RGB这种，但是你这个文件是BGR的，我们是不是要转一下？对不对
//		unsigned char temp = pixelData[i + 2]; //R
//		pixelData[i + 2] = pixelData[i];
//		pixelData[i] = temp;
//	}
//
//	// 3.数据已经被解析到pixelData中来了，接下里我们要做什么呢
//	// ?创建Opengl的纹理对象
//	GLuint texture; //创建纹理缓冲对象
//	glGenTextures(1, &texture);//告诉opengl仓库管理员，我要操作1号缓冲对象，是用来处理纹理数据的
//	glBindTexture(GL_TEXTURE_2D, texture);//告诉opengl仓库管理员，我一会儿药房数据到这个格子中
//
//	//ST VU 
//	// 过滤方式 warp mode
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);// V 垂直方向上超出[0,1]之后我们怎么处理，GL_CLAMP_TO_EDGE表示采样边缘杨色，不重复或者镜像
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// V 垂直方向上超出[0,1]之后我们怎么处理，GL_CLAMP_TO_EDGE表示采样边缘杨色，不重复或者镜像
//
//	// 缩放模式
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //线性 ,GL_NEAREST 最近邻采样
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	//没有minMapy
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	delete imgData;
//	return texture;
//
//}