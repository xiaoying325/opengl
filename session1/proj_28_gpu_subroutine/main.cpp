#include <windows.h>
#include <stdio.h>
#include "glew.h"
#include "misc.h"
#include "model.h"
#include "Glm/glm.hpp"
#include "Glm/ext.hpp"
#include "timer.h"
#include "frustum.h"


#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"glu32.lib")


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
	RECT rect;
	rect.left = 0;
	rect.right = 800;
	rect.bottom = 600;
	rect.top = 0;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	//如果注册成功了，那就开始创建窗口 调用CreateWindowEx函数
	HWND hwnd = CreateWindowEx(NULL, L"GLWindow", L"RenderWindow", WS_OVERLAPPEDWINDOW, 100, 100, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);


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
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;


	GL_CALL(glEnable(GL_LINEAR));

	// 初始化glew环境
	glewInit();

	//初始化字体
	InitFont(dc);

	GLuint program = CreateGPUProgram("res/shader/gpu_subroutine.vs", "res/shader/gpu_subroutine.fs");
	GLint posLocation, texcoordLocation, normalLocation, MLocation, VLocation, PLocation, NMLocation, textureLocation, offsetLocation, surfaceColorLocation;
	posLocation = glGetAttribLocation(program, "pos");
	texcoordLocation = glGetAttribLocation(program, "texcoord");
	normalLocation = glGetAttribLocation(program, "normal");
	offsetLocation = glGetAttribLocation(program, "offset");

	MLocation = glGetUniformLocation(program, "M");
	VLocation = glGetUniformLocation(program, "V");
	PLocation = glGetUniformLocation(program, "P");
	NMLocation = glGetUniformLocation(program, "NM");
	textureLocation = glGetUniformLocation(program, "U_MainTexture");


	surfaceColorLocation = glGetSubroutineUniformLocation(program, GL_FRAGMENT_SHADER, "U_SurfaceColor");


	GLuint amibentLightIndex = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "Ambient");
	GLuint diffuseLightIndex = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "Diffuse");
	GLuint specularLightIndex = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "Specular");


	Timer t;
	t.Start();
	unsigned int* indexes = nullptr;
	int vertexCount = 0, indexCount = 0;
	VertexData* vertexes = LoadObjModel("res/model/niutou.obj", &indexes, vertexCount, indexCount);
	if (vertexes == nullptr)
	{
		printf("LoadOBjModel Fail\n");
	}
	printf("load model cost %d\n", t.GetPassedTime());


	GLuint vbo = CreateBufferObject(GL_ARRAY_BUFFER, sizeof(VertexData) * vertexCount, GL_STATIC_DRAW, vertexes);
	GLuint ibo = CreateBufferObject(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, GL_STATIC_DRAW, indexes);
	//加载纹理bo
	GLuint mainTexture = CreateTextureFromFile("res/image/niutou.bmp");



	printf("vertex count %d index count %d\n", vertexCount, indexCount);
	glClearColor(0.1, 0.4, 0.6, 1.0);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);



	glm::mat4 model = glm::translate(0.0f, -0.5f, -10.0f) * glm::rotate(-90.0f, 0.0f, 1.0f, 0.0f) * glm::scale(0.01f, 0.01f, 0.01f);
	glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 normalMatrix = glm::inverseTranspose(model);


	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, width, height);


	MSG msg;

	DWORD lastTime = GetTickCount();
	int frameCount = 0;
	float fps = 0.0f;
	float angle = 0.0f;


	glm::vec3 positions[3];
	int rows = 4;
	int cols = 5;
	float spacingX = 2.0f;
	float spacingY = 2.0f;
	float startX = -((cols - 1) * spacingX) / 2.0f; // 让X居中
	float startY = ((rows - 1) * spacingY) / 2.0f;  // 让Y居中，注意Y轴向上

	for (int i = 0; i < 3; i++) {
		int row = i / cols;     // 当前行号 0~3
		int col = i % cols;     // 当前列号 0~4
		float x = startX + col * spacingX;
		float y = startY - row * spacingY;  // Y往下递减
		float z = -10.0f;  // 深度固定
		positions[i] = glm::vec3(x, y, z);
	}
	GLuint offsetVBO = CreateBufferObject(GL_ARRAY_BUFFER, sizeof(float) * 3*3, GL_STATIC_DRAW, positions);



	auto drawCommand = [&]()->void
	{
		glUseProgram(program); //指定shader


		glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(model)); //模型矩阵
		glUniformMatrix4fv(VLocation, 1, GL_FALSE, glm::value_ptr(view)); //视图矩阵 摄像机矩阵
		glUniformMatrix4fv(PLocation, 1, GL_FALSE, glm::value_ptr(projection)); // 投影矩阵  透视投影VS正交投影
		glUniformMatrix4fv(NMLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));//法线矩阵

		glBindTexture(GL_TEXTURE_2D, mainTexture); //绑定0号纹理单元
		glUniform1i(textureLocation, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);//绑定vbo
		glEnableVertexAttribArray(posLocation);
		glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
		glEnableVertexAttribArray(texcoordLocation);
		glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(normalLocation);
		glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 5));
		glBindBuffer(GL_ARRAY_BUFFER, 0); //解绑vbo


		model = glm::translate(-1.0f, -0.5f, -4.0f) * glm::rotate(-90.0f, 0.0f, 1.0f, 0.0f) * glm::scale(0.01f, 0.01f, 0.01f); //绘制第一个模型，指定光照shader
		glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &amibentLightIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);  // 绑定ibo
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // 解绑ibo


		model = glm::translate(0.0f, -0.5f, -4.0f) * glm::rotate(-90.0f, 0.0f, 1.0f, 0.0f) * glm::scale(0.01f, 0.01f, 0.01f);//绘制第二个模型，指定光照shader
		glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &diffuseLightIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); // 绑定ibo
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);// 解绑ibo


		model = glm::translate(1.0f, -0.5f, -4.0f) * glm::rotate(-90.0f, 0.0f, 1.0f, 0.0f) * glm::scale(0.01f, 0.01f, 0.01f); //绘制第三个模型，指定光照shader
		glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &specularLightIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); // 绑定ibo
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);// 解绑ibo



		glUseProgram(0); //解绑shader
	};


	//渲染主循环
	while (true)
	{
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //先擦除缓冲区，然后再重新绘制一帧的画面


		drawCommand();// 调用绘制指令
	

		/*
		glUseProgram(program);//指定接下来的绘制操作使用哪个着色器程序
		glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(VLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(PLocation, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(NMLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		glUniformMatrix4fv(VLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(PLocation, 1, GL_FALSE, glm::value_ptr(projection));

		
		glBindTexture(GL_TEXTURE_2D, mainTexture);// 绑定纹理操作缓冲区
		glUniform1i(textureLocation, 0);

		
		glBindBuffer(GL_ARRAY_BUFFER, vbo);// 绑定VBO操作缓冲区
		glEnableVertexAttribArray(posLocation);
		glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
		glEnableVertexAttribArray(texcoordLocation);
		glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(normalLocation);
		glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 5));
		glBindBuffer(GL_ARRAY_BUFFER, 0);// 解绑VBO操作缓冲区


		
		glBindBuffer(GL_ARRAY_BUFFER, offsetVBO);// 绑定顶点坐标偏移的VBO
		glEnableVertexAttribArray(offsetLocation); //启用顶点属性数组offsetLocation,告诉opengl仓库管理员，这个属性我们要用上，它对应shader中的offset
		glVertexAttribPointer(offsetLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0); // / 解绑顶点坐标偏移的VBO

		glVertexAttribDivisor(offsetLocation, 1);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);// 绑定IBO操作缓冲区
		glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0,3);// CPU通知GPU调用drawcall绘制指令
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);// 解绑IBO操作缓冲区
		glUseProgram(0);// 解绑我们的当前绘制操作指定的着色器程序

		
		
		*/



		
		SwapBuffers(dc);//交换缓冲区

	}

	return 0;
}