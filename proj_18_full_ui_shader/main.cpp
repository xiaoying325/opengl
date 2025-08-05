#include <windows.h>

#include "glew.h"
#include "Glm/glm.hpp"
#include "Glm/ext.hpp"
#include "misc.h"
#include "model.h"


#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glew32.lib")



#define _CRT_SECURE_NO_WARNINGS



//定义一个显示列表的基址
GLuint fontBase = 0;


// 提供一个绘制文本内容的函数
void InitFont(HDC dc) {
	HFONT font = CreateFont(
		-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
		L"Arial");
	SelectObject(dc, font);
	fontBase = glGenLists(96);
	wglUseFontBitmaps(dc, 32, 96, fontBase);

}


// 定义i一个绘制字体的函数，绘制以open左下角为起点
void RenderText(float x, float y, const char* text) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	// 设正交投影，坐标系左下角(0,0)，右上角(800,600)
	glOrtho(0, 800, 0, 600, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// 设置光栅位置，y轴从下往上增加，注意y是距离底部距离
	glRasterPos2f(x, y);

	glListBase(fontBase - 32);
	glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}



//struct  Vertex
//{
//	float pos[3];//xyz
//	float color[4];//RGAB
//};
//

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







// 创建着色器函数
GLuint CreateGPUProgram(const char* vsShaderPath, const char* fsShaderPath) {
	// 从磁盘上加载我们写好的着色器源码，并且创建为GPU可识别的着色器程序链接进去
	GLuint vsShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fsShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 要从磁盘上把我们写好的shader源码读取进来
	const char* vsCode = LoadFileContent(vsShaderPath);
	const char* fsCode = LoadFileContent(fsShaderPath);


	//editbin /subsystem:console "$(TargetPath)"


	std::cout << "vsShader: " << vsShader << std::endl;
	std::cout << "vsCode: " << (vsCode ? vsCode : "nullptr") << std::endl;


	std::cout << "fsShader: " << fsShader << std::endl;
	std::cout << "fsCode: " << (fsCode ? fsCode : "nullptr") << std::endl;


	//把读取到的GLSL源码上传到GPU
	glShaderSource(vsShader, 1, &vsCode, nullptr);
	glShaderSource(fsShader, 1, &fsCode, nullptr);

	//编译shader
	glCompileShader(vsShader);
	glCompileShader(fsShader);


	GLint Success;
	char infoLog[512];
	glGetShaderiv(vsShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(vsShader, 512, NULL, infoLog);
		MessageBoxA(NULL, infoLog, "Vertex Shader Compile Error", MB_OK | MB_ICONERROR);
	}
	// 检查 fsShader 编译错误
	glGetShaderiv(fsShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(fsShader, 512, NULL, infoLog);
		MessageBoxA(NULL, infoLog, "Fragment Shader Compile Error", MB_OK | MB_ICONERROR);
	}



	//创建一个shader程序模板，并且链接
	GLuint program = glCreateProgram();
	glAttachShader(program, vsShader);
	glAttachShader(program, fsShader);

	glLinkProgram(program);


	// 检查 program 链接错误
	glGetProgramiv(program, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		MessageBoxA(NULL, infoLog, "Program Link Error", MB_OK | MB_ICONERROR);
	}

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
	HWND hwnd = CreateWindowEx(NULL, L"GLWindow", L"RenderWindow", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);


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


	//定义我们视口大小的宽度和高度


	// 用这个api可以获取u我们opengl 绘制区的真是大小
	int width, height;
	RECT rect;

	GetClientRect(hwnd, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	// 800, 600,   难道不是他的视口宽度和高度么？

	// 初始化glew环境
	glewInit(); // 这是必须的

	//初始化字体
	InitFont(dc);

	// 创建两个GPU能够识别并使用的着色器程序
	GLuint  program = CreateGPUProgram("res/shader/ui_full.vs","res/shader/ui_full.fs");

	// 上面这一步我们已经完成GPU程序创建
	GLint posLocation, texcoordLocation,normalLocation,MLocation, VLocation, PLocation,NMLocation,textureLocation; //定义法线矩阵的变量
	// ？如何从着色器中拿到这些变量呢？
	posLocation = glGetAttribLocation(program, "pos");
	texcoordLocation = glGetAttribLocation(program, "texcoord");
	normalLocation = glGetAttribLocation(program, "normal");

	MLocation = glGetUniformLocation(program, "M");
	VLocation = glGetUniformLocation(program, "V");
	PLocation = glGetUniformLocation(program, "P");
	NMLocation = glGetUniformLocation(program, "NM");
	textureLocation = glGetUniformLocation(program, "U_MainTexture"); 


	//  0(R) 0(G) 0(B)  0(A) 

	unsigned int* indexes = nullptr;
	int vertexCount = 0, indexCount = 0;
	VertexData* vertexes = LoadObjModel("res/model/Quad.obj", &indexes, vertexCount, indexCount);
	if (vertexes == nullptr)
	{
		printf("LoadOBjModel Fail\n");
	}

	printf("LoadObjModel Success!!!!!!!!!!!:%s\n", vertexes);



	GLuint vbo = CreateBufferObject(GL_ARRAY_BUFFER, sizeof(VertexData) * vertexCount, GL_STATIC_DRAW, vertexes);
	GLuint ibo = CreateBufferObject(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, GL_STATIC_DRAW, indexes);
	//加载纹理bo
	GLuint mainTexture = CreateTextureFromFile("res/image/niutou.bmp");
	//GLuint mainTexture = CreateTextureFromFile("res/image/150001.dds");



	printf("vertex count %d index count %d\n", vertexCount, indexCount);
	glClearColor(0.1, 0.4, 0.6, 1.0); //蓝色


	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	glEnable(GL_DEPTH_TEST); //开启深度测试，

	float identity[] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	glm::mat4 model = glm::translate(0.0f, 0.0f, -100.0f); // 我们把模型推到摄像机前面-4的位置了
	glm::mat4 projection = glm::perspective(45.0f, (float)width /(float)height, 0.1f, 1000.0f); //透视投影矩阵
	glm::mat4 normalMatrix = glm::inverseTranspose(model);

	glEnable(GL_BLEND);//开启混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // z这个起始i就是标准的Alpha混合公式

	// 设置我们的视口大小
	glViewport(0, 0, width, height);

	//定义一个消息
	MSG msg;

	//计算帧率变量
	DWORD lastTime = GetTickCount();
	int frameCount = 0;
	float fps = 0.0f;

	float angle = 0.0f;

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



		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //使用背景色清除颜色缓冲 !!!!如果开启了深度测试，比如在每一帧绘制子花钱，清理深度缓冲区
		glUseProgram(program);


		//矩阵赋值
		glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(VLocation, 1, GL_FALSE, identity);
		glUniformMatrix4fv(PLocation, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(NMLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));


		glActiveTexture(GL_TEXTURE0);//显示激活纹理单元0号，为默认都是激活的
		glBindTexture(GL_TEXTURE_2D, mainTexture); //表示我接下来要使用这个纹理进行绘制
		glUniform1i(textureLocation, 0); //给shader的祝文里字段赋值  0号纹理单元

	
		//绑定VBO和给着色器变量赋值
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(posLocation);
		glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
		glEnableVertexAttribArray(texcoordLocation);
		glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(normalLocation);
		glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 5));

		//调用DrawCallCommand
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//解绑着色器
		glUseProgram(0);




		//计算帧率和计算每一帧绘制的定点数以及三角形数目
		frameCount++;
		DWORD currentTime = GetTickCount();
		if (currentTime - lastTime >= 1000) {
			fps = frameCount * 1000.0f / (currentTime - lastTime);
			frameCount = 0;
			lastTime = currentTime;
		}

		//绘制文字信息
		char info[128];
		int vertextCount = vertexCount;
		int triangleCount = vertextCount / 3;
		sprintf(info, "FPS: %.2f  Vertices: %d   Indexes: %d  Triangles: %d", fps, vertextCount, indexCount, triangleCount);
		// 沪指的HUDUi，所以我们要关闭深度测试
		glDisable(GL_DEPTH_TEST);
		glColor3f(1.0f, 1.0f, 1.0f);

		//左下角绘制
		RenderText(10.0f, 10.0f, info);
		glEnable(GL_DEPTH_TEST);
		


		//交换缓冲区
		SwapBuffers(dc);

	}

	return 0;
}