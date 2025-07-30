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
		-24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
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
		100, 100, 800, 600, NULL, NULL, hInstance, NULL);


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

	//初始化字体
	InitFont(dc);

	// 创建两个GPU能够识别并使用的着色器程序
	GLuint  program = CreateGPUProgram("res/shader/vs.shader","res/shader/fs.shader");

	// 上面这一步我们已经完成GPU程序创建
	GLint posLocation, texcoordLocation,normalLocation,MLocation, VLocation, PLocation;
	// ？如何从着色器中拿到这些变量呢？
	posLocation = glGetAttribLocation(program, "pos");
	texcoordLocation = glGetAttribLocation(program, "texcoord");
	normalLocation = glGetAttribLocation(program, "normal");

	MLocation = glGetUniformLocation(program, "M");
	VLocation = glGetUniformLocation(program, "V");
	PLocation = glGetUniformLocation(program, "P");


	//顶点数据结构

	// 这是不是第一个顶点？
	//Vertex vertex[3];
	//vertex[0].pos[0] = 0;   //0
	//vertex[0].pos[1] = 0;
	//vertex[0].pos[2] = -100.0f;

	//vertex[0].color[0] = 1.0f;
	//vertex[0].color[1] = 1.0f;
	//vertex[0].color[2] = 1.0f;
	//vertex[0].color[3] = 1.0f;


	//// 这是不是第二个顶点？

	//vertex[1].pos[0] = 10;    //1
	//vertex[1].pos[1] = 0;
	//vertex[1].pos[2] = -100.0f;

	//vertex[1].color[0] = 1.0f;
	//vertex[1].color[1] = 1.0f;
	//vertex[1].color[2] = 1.0f;
	//vertex[1].color[3] = 1.0f;


	//// 这是不是第三个顶点？

	//vertex[2].pos[0] = 0;
	//vertex[2].pos[1] = 10;
	//vertex[2].pos[2] = -100.0f; //2

	//vertex[2].color[0] = 1.0f;
	//vertex[2].color[1] = 1.0f;
	//vertex[2].color[2] = 1.0f;
	//vertex[2].color[3] = 1.0f;


	// 使用vbo
	//GLuint vbo;
	//// 创建缓冲对象ID
	//glGenBuffers(1, &vbo);

	//// 绑定缓冲到目标中
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);

	////分配显存并上传数据到现存
	//glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3, vertex, GL_STATIC_DRAW);

	//// 解绑缓冲区
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//???为什么要使用vbo？
	//为了节省内存以及性能





	// IBO 什么是IBO ，又称之为EBO 
	// 为什么我们要使用这个玩意？
	// 
	/*
		Vertex vertices[] = {
			{pos0,color0}
			{pos1,color1}
			{pos2,color2}
		}

		glDrawArrays(GL_TRIANGLES,0,3)
		//这样起始没问题，因为你只有一个三角形，但是问题就来了，当你要绘制一个正方形得时候，你怎么办？
		// 正方向它由两个三角形组成
		0    1

		2    3

	
	    第一个三角形顶点如果仍然使用VBO来存储，就是0，1，2
		第二个三角形顶点如果仍然使用VBO来村村，就是2，1，3

		0，1，2，2，1，3  一共写了6个顶点数据，其中12顶点是重复的，
		glDrawArrays(GL_TRIANGLES,0,3)

		出现了IBO，

		IBO它只是存储了绘制时你要使用的顶点索引顺序
		VBO  0 1 2 3
		IBO 0 1 2 2 1 3



		如果我们使用IBO来渲染，他的流程是：？？？？
		读取IBO 第一个索引0 ，取VBO[0]
		//以此类推
		最终得到两个三角形的顶点数据

		// 首先我们要声明IBO
		// 定义号IBO存储顶点的顺序
		// 把申请的IBO解绑

		// 绘制时我们使用glElements(GL_TRIANGLES，3，)


	*/


	//unsigned int indexes[] = { 0,1,2 };

	//GLuint vbo = CreateBufferObject(GL_ARRAY_BUFFER, sizeof(Vertex) * 3, GL_STATIC_DRAW, vertex);
	//GLuint ibo = CreateBufferObject(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3, GL_STATIC_DRAW, indexes);



	//GLuint ibo;
	//glGenBuffers(1, &ibo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	////分配显存并上传数据到现存
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3, indexes, GL_STATIC_DRAW);

	//// 解绑缓冲区
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);






	/*
	
	要绘制模型，我们该怎么办呢？
	1.是不是要加载模型？
	2.加载完是不是解析模型的数据？
	3.解析完，我们是不是要按照之前说的知识点，把解析之后的模型数据，如何组成VBO IBO 以及如何调用OPENGL进行绘制？


	
	*/



	unsigned int* indexes = nullptr;
	int vertexCount = 0, indexCount = 0;
	VertexData* vertexes = LoadObjModel("res/model/niutou.obj", &indexes, vertexCount, indexCount);
	if (vertexes == nullptr)
	{
		printf("LoadOBjModel Fail\n");
	}

	printf("LoadObjModel Success!!!!!!!!!!!:%s\n", vertexes);


	GLuint vbo = CreateBufferObject(GL_ARRAY_BUFFER, sizeof(VertexData) * vertexCount, GL_STATIC_DRAW, vertexes);
	GLuint ibo = CreateBufferObject(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, GL_STATIC_DRAW, indexes);

	printf("vertex count %d index count %d\n", vertexCount, indexCount);

	// 指定下我们清屏时所使用的颜色
	glClearColor(0.1, 0.4, 0.6, 1.0);




	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);



	float identity[] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	glm::mat4 model = glm::translate(0.0f, 0.0f, -3.0f) * glm::rotate(-45.0f, 0.0f, 1.0f, 0.0f) * glm::scale(0.8f, 0.8f, 0.8f);;
	glm::mat4 projection = glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 1000.0f);





	//定义一个消息
	MSG msg;

	//计算帧率变量
	DWORD lastTime = GetTickCount();
	int frameCount = 0;
	float fps = 0.0f;


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
		glUseProgram(program);


		// 把三个矩阵进行赋值

		glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(VLocation, 1, GL_FALSE, identity);
		glUniformMatrix4fv(PLocation, 1, GL_FALSE, glm::value_ptr(projection));

		// OpenGL渲染状态机的概念+ 逐帧刷新绘制
		// 什么是状态机，为什么把Opengl的绘制过程称作为Opengl状态机？？
		// 状态机 指的是一个系统在运行过程中能够保持唯一的一组状态，或者一个状态，所有操作都是修改或使用这些状态，而不是直接指定操作结果
		// 在opengl中GPU渲染行为完全依赖于当前的上下文状态 Context State
		// 我们在拍照的时候，拍照之前，（绘制之前），是不是要设置快门，镜头参数，曝光度，以及是什么模式，还有焦距。。。
		// 当我们设置好了之后，我们要干啥？就是按下快门拍照，（这个是不是可以理解为绘制）
		// 如果我们手拿着摄像机，我就拍摄面前这个物体，我就用上面设置的哪些全局参数设计，进行拍照，起始理论上来说，这些状态可以不必每帧都重新在设置一般，起始可以复用
		// 但这个是理想状态，在游戏世界中，连续动态，所以我们每一帧都需要重新来设置这些全局状态参数，以确保每一帧画面都是符合我们预期的？你可以想象一下，你玩游戏时候，看到的动态可连续的画面

		//

		//glBindBuffer(GL_ARRAY_BUFFER, vbo);
		////接下来我们从vbo中读取数据
		////  glEnableVertexAttribArray(posLocation);  它是一种规则，什么规则？告诉opengl，从vbo中解析顶点数据的规则，我们可以看到它是Array
		//glEnableVertexAttribArray(posLocation);
		//// 这句话的意思是说，从偏移0开始，每隔sizeof（Vertext）字节数据开始，取3个float 作为位置属性
		//glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*)0); //下面告诉你了，如何解析vbo中传递进来的所有顶点数据
		//glEnableVertexAttribArray(colorLocation);
		//glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*3));



		////解绑当前opengl中绑定的vbo对象
		//glBindBuffer(GL_ARRAY_BUFFER, 0); //这一帧我们用完了解绑当前帧 之前绑定vbo数据
		////调用绘制指令 ,这种是没有使用IBO的绘制方法
		////glDrawArrays(GL_TRIANGLES, 0, 3);

		//// 使用IBO的绘制方法

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);




		// 模型的那一陀数据已经被我们解析成了IBO和VBO
		//接下来我们就开始画画呗
	
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(posLocation);
		glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
		glEnableVertexAttribArray(texcoordLocation);
		glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(normalLocation);
		glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float) * 5));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


		// 调用shader是默认不变的
		glUseProgram(0);


		//计算帧率和计算每一帧绘制的定点数以及三角形数目
		//frameCount++;
		//DWORD currentTime = GetTickCount();
		//if (currentTime - lastTime >= 1000) {
		//	fps = frameCount * 1000.0f / (currentTime - lastTime);
		//	frameCount = 0;
		//	lastTime = currentTime;
		//}

		////绘制文字信息
		//char info[128];
		//int vertextCount = 3;
		//int triangleCount = vertextCount / 3;
		//sprintf(info, "FPS: %.2f  Vertices: %d  Triangles: %d", fps, vertextCount, triangleCount);
		//// 沪指的HUDUi，所以我们要关闭深度测试
		//glDisable(GL_DEPTH_TEST);
		//glColor3f(1.0f, 1.0f, 1.0f);

		////左下角绘制
		//RenderText(10.0f, 10.0f, info);
		//glEnable(GL_DEPTH_TEST);
		//




		SwapBuffers(dc);

	}

	return 0;
}