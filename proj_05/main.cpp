#include <windows.h>
#include <stdio.h>
#include "glew.h"

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glew32.lib")



struct  Vertex
{
	float pos[3];//xyz
	float color[4];//RGAB
};


LRESULT CALLBACK GLWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE://点击了关闭窗口的按钮
		PostQuitMessage(0);//发一个WM_QUIT消息给程序
		break;
	}

	//window系统提供的，默认的用来处理消息的函数
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


char* LoadFileContent(const char*path) {
	FILE* pFile = fopen(path, "rb"); // rb read binary 格式去读取
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END); // 把文件指针移动到文件末尾
		int nLen = ftell(pFile);// 这个文件有多大啊
		char* buffer = new char[nLen + 1];//创建一个这样大小的buffer数组，这里为什么加1  因为我们要多写入一个\0, “Hellow” H e l l o \0
		rewind(pFile);// 把文件指针再次移动到文件开头
		fread(buffer, nLen, 1, pFile);//开始读取内容
		buffer[nLen] = '\0';//读取完成之后，我们还要在末尾加一个\0
		fclose(pFile);//读完完成关闭文件句柄
		return buffer;
	}
	fclose(pFile);
	return nullptr;
}




// 创建着色器函数
GLuint CreateGPUProgram(const char* vsShaderPath, const char* fsShaderPath) {
	// 从磁盘上加载我们写好的着色器源码，并且创建为GPU可识别的着色器程序链接进去
	GLuint vsShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fsShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 要从磁盘上把我们写好的shader源码读取进来
	const char* vsCode = LoadFileContent(vsShaderPath);
	const char* fsCode = LoadFileContent(fsShaderPath);

	//把读取到的GLSL源码上传到GPU
	glShaderSource(vsShader, 1, &vsCode, nullptr);
	glShaderSource(fsShader, 1, &fsCode, nullptr);

	//编译shader
	glCompileShader(vsShader);
	glCompileShader(fsShader);

	//创建一个shader程序模板，并且链接
	GLuint program = glCreateProgram();
	glAttachShader(program, vsShader);
	glAttachShader(program, fsShader);

	glLinkProgram(program);

	//删除
	glDetachShader(program, vsShader);
	glDetachShader(program, fsShader);


	glDeleteShader(vsShader);
	glDeleteShader(fsShader);


	return program;


}


INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wndclass;
	wndclass.cbClsExtra = 0;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);//给鼠标一个系统内置的图标
	wndclass.hIcon = NULL;
	wndclass.hIconSm = NULL;
	wndclass.hInstance = hInstance;

	//消息相应函数，例如鼠标、键盘对窗口进行了操作，如何响应？就是这个函数
	wndclass.lpfnWndProc = GLWindowProc;
	wndclass.lpszClassName = L"GLWindow";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_VREDRAW | CS_HREDRAW; //水平重绘 垂直重绘

	//上面窗口的基本参数已经构建完毕，开始注册这个窗口
	ATOM atom = RegisterClassEx(&wndclass);
	if (!atom)
	{
		return 0; //如果窗口没有注册成功，就直接退出程序
	}


	//如果注册成功了，那就开始创建窗口 调用CreateWindowEx函数
	HWND hwnd = CreateWindowEx(NULL, L"GLWindow", L"OpenGL Window", WS_OVERLAPPEDWINDOW,
		100, 100, 640, 960, NULL, NULL, hInstance, NULL);


	//初始化opengl渲染绘图环境
	HDC dc = GetDC(hwnd);//获取当前设备的上下文，

	// 绑定当前窗口的像素格式
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	//
	pfd.nVersion = 1;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);

	// 设置下缓冲区的精度
	pfd.cColorBits = 32; //颜色缓冲  32位，RGBA 一个通道占用4个字节 
	pfd.cDepthBits = 24;//深度缓冲，24位，z-buffer,用于深度测试
	pfd.cStencilBits = 8;//模板缓冲

	// if（《0.5） disCard

	// 像素类型 和层级
	pfd.iPixelType = PFD_TYPE_RGBA; //每个像素独立存储的eRGABA值
	pfd.iLayerType = PFD_MAIN_PLANE; // 普通绘图平面，不是覆盖层或者叠加层

	//设置渲染模式i标志

	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; //PFD_DRAW_TO_WINDOW 表示绘制目标是窗口  PFD_SUPPORT_OPENGL 支持OPENGL渲染，PFD_DOUBLEBUFFER 表示我们绘制时使用的是双缓冲

	// 选择并且应用像素格式
	int pixelFormat = ChoosePixelFormat(dc, &pfd);
	SetPixelFormat(dc, pixelFormat, &pfd);


	//初始化opengl的绘图上下文  必须 必须  必须
	HGLRC rc = wglCreateContext(dc);
	wglMakeCurrent(dc, rc);

	// 初始化glew环境
	glewInit(); // 这是必须的

	// 创建两个GPU能够识别并使用的着色器程序
	GLuint  program = CreateGPUProgram("vs.shader","fs.shader");

	// 上面这一步我们已经完成GPU程序创建
	GLint posLocation, colorLocation, MLocation, VLocation, PLocation;
	// ？如何从着色器中拿到这些变量呢？
	posLocation = glGetAttribLocation(program, "pos");
	colorLocation = glGetAttribLocation(program, "color");

	MLocation = glGetUniformLocation(program, "M");
	VLocation = glGetUniformLocation(program, "V");
	PLocation = glGetUniformLocation(program, "P");


	//顶点数据结构
	Vertex vertex[3];
	vertex[0].pos[0] = 0;
	vertex[0].pos[1] = 0;
	vertex[0].pos[2] = -100.0f;

	vertex[0].color[0] = 1.0f;
	vertex[0].color[1] = 1.0f;
	vertex[0].color[2] = 1.0f;
	vertex[0].color[3] = 1.0f;

	vertex[1].pos[0] = 10;
	vertex[1].pos[1] = 0;
	vertex[1].pos[2] = -100.0f;

	vertex[1].color[0] = 1.0f;
	vertex[1].color[1] = 1.0f;
	vertex[1].color[2] = 1.0f;
	vertex[1].color[3] = 1.0f;

	vertex[2].pos[0] = 0;
	vertex[2].pos[1] = 10;
	vertex[2].pos[2] = -100.0f;

	vertex[2].color[0] = 1.0f;
	vertex[2].color[1] = 1.0f;
	vertex[2].color[2] = 1.0f;
	vertex[2].color[3] = 1.0f;


	// 使用vbo
	GLuint vbo;
	// 创建缓冲对象ID
	glGenBuffers(1, &vbo);

	// 绑定缓冲到目标中
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//分配显存并上传数据到现存
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3, vertex, GL_STATIC_DRAW);

	// 解绑缓冲区
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//???为什么要使用vbo？
	//为了节省内存以及性能







	// 指定下我们清屏时所使用的颜色
	glClearColor(0.1, 0.4, 0.6, 1.0);




	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);


	//定义一个消息
	MSG msg;

	//定义一个死循环，用来抓取消息（游戏引擎的主循环） 1s估计能刷66多帧
	while (true)
	{
		//如果消息来了
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			//如果是退出消息，那就直接退出，跳出while
			if (msg.message == WM_QUIT)
			{
				break;
			}
			//如果非退出消息，那就把消息转换一下，并底层派发出去
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}


		//接入每帧opengl的绘制逻辑
		glClear(GL_COLOR_BUFFER_BIT); //使用背景色清除颜色缓冲

		SwapBuffers(dc);

	}

	return 0;
}