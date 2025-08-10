#include "misc.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <functional>





GLuint CreateFrameBufferObject(GLuint& colorBuffer, GLuint& depthBuffer, int width, int height) {
	//1.创建fbo
	GLuint fbo;
	// 简化理解：像工厂给你发了一个唯一编号（ID），你以后操作这个编号代表的Framebuffer。1 就是说你要这个工厂给你1个编号
	glGenFramebuffers(1, &fbo);// 生成一个framebuffer对象，fbo是他的id   只能生成一个id，然后绑定，配置它 1 生成一个，第二参数是指针，告诉opengl把生成的id写到那里，这里我们是写到fbo中
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);//绑定该fbo，后续操作都针对它进行


	// 生成颜色缓冲纹理
	glGenTextures(1, &colorBuffer);               // 3. 生成一个纹理ID，保存在colorBuffer变量中
	glBindTexture(GL_TEXTURE_2D, colorBuffer);    // 4. 绑定该纹理对象
	// 5. 设置纹理参数：线性过滤，纹理坐标超出范围时边缘延伸（CLAMP_TO_EDGE）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// 6. 分配纹理内存，格式为RGBA8（8位RGBA），宽高由参数决定，数据为空（nullptr）
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);              // 7. 解绑纹理，防止误操作
	// 8. 将该纹理附加到当前绑定的Framebuffer的颜色附件0上
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);


	// 生成深度缓冲纹理附件
	glGenTextures(1, &depthBuffer);                // 9. 生成一个纹理ID，保存在depthBuffer变量中
	glBindTexture(GL_TEXTURE_2D, depthBuffer);     // 10. 绑定该纹理对象
	// 11. 设置纹理参数，同颜色纹理
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// 12. 分配深度纹理内存，格式为24位深度，宽高由参数决定，数据为空
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);               // 13. 解绑纹理
	// 14. 将该深度纹理附加到当前Framebuffer的深度附件上
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);


	// 检查下framebuffer的完整性，确保所有附近都是正常的
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		// 16. 如果不完整，打印错误信息，并释放已申请的资源，返回0表示失败
		printf("create framebuffer object fail: 0x%x\n", status);
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &colorBuffer);
		glDeleteTextures(1, &depthBuffer);
		fbo = 0;
	}

	// 17. 解绑Framebuffer，切换回默认Framebuffer（窗口系统提供的）
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 18. 返回创建的Framebuffer对象ID（或失败时0）
	return fbo;


}


GLuint CreateVAOWithVBOSettings(std::function<void()>recordSetting)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	if (recordSetting) {
		recordSetting(); //把操作行为给记录下来，记录到vao中， 就不需要每次重新操作哪些重复的操作了
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
