#include "model.h"
#include "misc.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>


VertexData* LoadObjModel(const char* filePath, unsigned int** indexes, int& vertexCount, int& indexCount) {
	// 这节课的重点就是接触这个模型的数据，最终返回的结构，是我们约束好的结构
	// 第一步 ，读取模型文件
	char* fileContent = LoadFileContent(filePath);

	if (!fileContent) {
		printf("Failed Load Obj File:%s\n", filePath);
		return nullptr;
	}


	printf("Obj Load Success\n%s\n", fileContent);



	// 第二步，我们要解析了
	struct  VertexInfo
	{
		float v[3];
	};

	std::vector<VertexInfo> positions;//我们要解析的坐标数据
	std::vector<VertexInfo> texcoords;//我们要解析的纹理坐标数据
	std::vector<VertexInfo> normals;//我们要解析的法线坐标数据

	// 怎么解？可以用stringstream解析文件,使用stringsre创建了一个字符串流ssObjFile，把fileContent的内容也就是Obj的内容，全部传了进去，这样
	// 方便那我们后续解流操作
	std::stringstream ssObjFile(fileContent);
	std::string line;// 每一行的内容
	std::string temp; // temo可以帮助我们跳过不需要的内容


	while (std::getline(ssObjFile,line))
	{
		if (line.empty() || line[0] == '#') {
			continue;
		}
		std::stringstream ssOneLine(line);
		char type = line[0];
		if (type == 'v') {

			// v里面又分为v vt vn 对这三类数据再进行解析
			if (line.size() > 1 && line[1] == 't') {
				// 纹理坐标 我们只需要读取2个数，因为纹理坐标也叫UV坐标 只有两个数
				ssOneLine >> temp;  // vt 0.000000 0.000000  我们把这段数据切成一段段，>》temp标识 从这u但刘数据中，拿出第一个词 存进temp
				// 为什么呢？因为我么nOBj文件每一行开头都有关键词，先读出来，我么你之打破这一行是什么数据
				// 我们的作用主要是为了跳过这个词，读取后面的内容

				// vt 0.000000 0.000000
				VertexInfo vi{};
				ssOneLine >> vi.v[0] >> vi.v[1]; // 0.000000 0.000000
				texcoords.push_back(vi);
				printf("[vt] %f,%f\n", vi.v[0], vi.v[1]);

			}
			else if (line.size() > 1 && line[1] == 'n') {
				//法线信息
				// vn: 法线
				ssOneLine >> temp;
				VertexInfo vi{};
				ssOneLine >> vi.v[0] >> vi.v[1] >> vi.v[2]; // 
				normals.push_back(vi);
				printf("[vn] %f, %f, %f\n", vi.v[0], vi.v[1], vi.v[2]);
			}
			else {
				//坐标信息
				 // v: 顶点坐标
				ssOneLine >> temp;
				VertexInfo vi{};
				ssOneLine >> vi.v[0] >> vi.v[1] >> vi.v[2];
				positions.push_back(vi);
				printf("[v] %f, %f, %f\n", vi.v[0], vi.v[1], vi.v[2]);
			}
		}
		else if (type == 'f') {
			//TODO 解面信息
			// 面数据，目前只是输出
			printf("[f] %s\n", line.c_str());
			// TODO: 解析 f -> 索引数组
		}


	}

	delete[] fileContent;  // 释放文件缓存
	return nullptr;

}