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


	GLuint vao = CreateVAOWithVBOSettings([&]()->void
		{
			GLuint vbo = CreateBufferObject(GL_ARRAY_BUFFER, sizeof(VertexData) * vertexCount, GL_STATIC_DRAW, vertexes);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glEnableVertexAttribArray(posLocation);
			glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
			glEnableVertexAttribArray(texcoordLocation);
			glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 3));
			glEnableVertexAttribArray(normalLocation);
			glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 5));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		});


	GLuint ibo = CreateBufferObject(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, GL_STATIC_DRAW, indexes);
	//加载纹理bo
	GLuint mainTexture = CreateTextureFromFile("res/image/niutou.bmp");



	printf("vertex count %d index count %d\n", vertexCount, indexCount);

;

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

	// 1. 创建4个纹理和4个FBO，纹理大小可与窗口相同
	GLuint colorBuffers[4];
	GLuint fbos[4];
	const int texWidth = 800;
	const int texHeight = 600;


	for (int i = 0; i < 4; ++i) {
		// 创建纹理
		glGenTextures(1, &colorBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 创建FBO
		glGenFramebuffers(1, &fbos[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[i], 0);

		// 简单不附加深度缓冲，这里根据需要添加

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("FBO %d incomplete\n", i);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //解绑





	
// 2. 创建一个简单屏幕四边形VAO，用于绘制每个小区域，后面会用model矩阵偏移到不同区域
	float quadVertices[] = {
		// posX, posY, texX, texY
		-0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 1.0f,
	};
	unsigned int quadIndices[] = { 0,1,2, 2,3,0 };

	


	GLuint quadVAO, quadVBO, quadEBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glGenBuffers(1, &quadEBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	// pos attribute loc=0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	// texcoord attribute loc=1
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);


	GLuint screenProgram = CreateGPUProgram("res/shader/fbo.vs", "res/shader/fbo.fs");
	GLuint screenTextureLocation = glGetUniformLocation(screenProgram, "screenTexture");


	// 3. 你已有的shader程序，假设名为 screenProgram，位置uniform名 modelLoc等
// 这里需要4个不同model矩阵，将屏幕四边形分别放置4个区块

	glm::mat4 models[4];
	// 计算4个区域model矩阵，缩放为屏幕1/2大小，平移到四个角
// 屏幕中心是(0,0)，坐标范围x,y都是[-1,1]
// 因此半屏大小是0.5f

	float halfSize = 0.5f; // 四边形原始大小是1，缩0.5后是0.5

	models[0] = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(halfSize, halfSize, 1.0f)); // 左下
	models[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(halfSize, halfSize, 1.0f)); // 右下
	models[2] = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(halfSize, halfSize, 1.0f)); // 左上
	models[3] = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(halfSize, halfSize, 1.0f)); // 右上




// --- 你的 drawCommand 增加一个参数，来区分第几个FBO渲染 ---
	auto drawCommand = [&](int index)->void
	{
		glUseProgram(program); // 使用你的模型shader

		// 这里根据index可以改变模型变换或光照，示例简单用不同位置
		glm::mat4 localModel = model;
		if (index == 0) {
			localModel = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -0.5f, -4.0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.01f));
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &amibentLightIndex);
		}
		else if (index == 1) {
			localModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -4.0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.01f));
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &diffuseLightIndex);
		}
		else if (index == 2) {
			localModel = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -0.5f, -4.0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.01f));
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &specularLightIndex);
		}
		else {
			// 你可以自定义第4个的绘制内容，这里就用第一个模型示例
			localModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.01f));
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &amibentLightIndex);
		}

		// 计算对应的法线矩阵
		glm::mat4 normalMatrix = glm::inverseTranspose(localModel);

		// 设置uniform
		glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(localModel));
		glUniformMatrix4fv(VLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(PLocation, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(NMLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		// 绑定纹理，绑定0号纹理单元
		glBindTexture(GL_TEXTURE_2D, mainTexture);
		glUniform1i(textureLocation, 0);

		glBindVertexArray(vao);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
		glUseProgram(0);
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

		// 1. 分别绑定4个FBO，渲染不同内容
		for (int i = 0; i < 4; ++i) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
			glViewport(0, 0, texWidth, texHeight);
			glClearColor(0.2f * i, 0.3f, 0.4f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			drawCommand(i); // 传入index，区分绘制
		}

		// 2. 解绑FBO，切回默认帧缓冲，绘制4个四边形显示4个纹理
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 800, 600);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(screenProgram);

		// 你这儿如果shader有view、proj也一并传，通常设为单位矩阵即可
		glm::mat4 identity = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(screenProgram, "view"), 1, GL_FALSE, glm::value_ptr(identity));
		glUniformMatrix4fv(glGetUniformLocation(screenProgram, "projection"), 1, GL_FALSE, glm::value_ptr(identity));

		for (int i = 0; i < 4; ++i) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glUniform1i(glGetUniformLocation(screenProgram, "screenTexture"), 0);

			glUniformMatrix4fv(glGetUniformLocation(screenProgram, "model"), 1, GL_FALSE, glm::value_ptr(models[i]));

			glBindVertexArray(quadVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);

		glUseProgram(0);

		SwapBuffers(dc);

	}

	return 0;
}