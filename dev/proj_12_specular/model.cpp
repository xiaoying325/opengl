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


	//定义一个存储解析出来的面的结构体，用来干啥呢？
	//用来标识每个面的每个顶点使用了哪些数据，我们已知有pos，texcoord normal
	// v/t/n ----> postionIndex /texcoordIndex /normalIndex
	struct  VertextDefine
	{
		int positionIndex;
		int texcoordIndex;
		int normalIndex;
	};


	std::vector<VertexInfo> positions;//我们要解析的坐标数据
	std::vector<VertexInfo> texcoords;//我们要解析的纹理坐标数据
	std::vector<VertexInfo> normals;//我们要解析的法线坐标数据


	// 解析出的顶点数据
	std::vector<VertextDefine> vertices;
	std::vector<unsigned int >objIndexes;// IBO

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
			// 这里我们只解三角形面
			// 第一步 去掉f
			ssOneLine >> temp; //f 1/1/1 2/2/2 3/3/3  ---> 1/1/1 2/2/2 3/3/3
			std::string vertexStr;

			for (int i = 0; i < 3; i++) {
				ssOneLine >> vertexStr;
				size_t pos = vertexStr.find_first_of("/");// 1
				std::string postionIndexStr = vertexStr.substr(0, pos);// 截取的终点不包含pos

				// 找到第二个/的索引位置
				size_t pos2 = vertexStr.find_first_of("/", pos + 1);
				std::string texcoordIndexStr = vertexStr.substr(pos + 1, pos2 - pos - 1); //pos2 在索引=3的位置， 截取的长度是1
				std::string normalIndexStr = vertexStr.substr(pos2 + 1, vertexStr.length() - pos2 - 1);

				//把顶点的数据从obj文本中解析出来，但是这里有个问题，就是这个解析出来的index需要转换下
				// 这里为什么要转换，我刚一直也描述过了
				// 顶点数组，postions【0】 positons【1】 positons[2]
				// Obj文件中 f 1/2/3  --->其实像访问的是，postions[0] texcoord[1] normal[2]
				VertextDefine vd;
				vd.positionIndex = atoi(postionIndexStr.c_str()) - 1; //atoi 转成一个int类型 原先是字符串1，转成int类型1 -1；
				vd.texcoordIndex = atoi(texcoordIndexStr.c_str()) - 1;
				vd.normalIndex = atoi(normalIndexStr.c_str()) - 1;

				//判断下我们顶点数组中是否已经保存了我们解析出来的这个顶点，允许复用，可能存储在重复的顶点，如果重复的我么你就服用
				int nCurrentIndex = -1;
				size_t nCurrentVerticeCount = vertices.size();
				for (size_t j = 0; j < nCurrentVerticeCount; j++)
				{
					if (vertices[j].positionIndex == vd.positionIndex &&
						vertices[j].texcoordIndex == vd.texcoordIndex &&
						vertices[j].normalIndex == vd.normalIndex)
					{
						nCurrentIndex = j;
						break;
					}
				}


				/*
					索引0 --》（0，0，0）
					索引1--》 （1，1，1）

					（2，2，2）

				*/
				if (nCurrentIndex == -1) {
					nCurrentIndex = vertices.size(); //2
					vertices.push_back(vd);
				}

				//可以把顶点的索引存入到IBO中
				/*
				f 1/1/1 2/2/2 3/3/3
				f 3/3/3 2/2/2 4/4/4
				6个顶点，但是其中是有重读的，为什么之所以定义下面这两个东西，就是i为了优化
				
				std::vector<VertextDefine> vertices;//VBO
				std::vector<unsigned int >objIndexes;// IBO

				vertextes里面经过我们上面去重逻辑之后的顶点数据为
				索引 0 -> (1,1,1)
				索引 1 -> (2,2,2)
				索引 2 -> (3,3,3)
				索引 3 -> (4,4,4)

				再看下IBO的数据
				objIndexes = [0,1,2,2,1,3] //构成的两个三角形面


				*/
				objIndexes.push_back(nCurrentIndex);

			}
		}
	}



	//返回下IBO和VBO
	indexCount = (int)objIndexes.size();
	*indexes = new unsigned int[indexCount];
	for (int i = 0; i < indexCount; i++) {
		(*indexes)[i] = objIndexes[i];
	}

	//构成VBO
	vertexCount = (int)vertices.size();
	VertexData* vertexes = new VertexData[vertexCount];
	for (int i = 0; i < vertexCount; ++i) {
		memcpy(vertexes[i].position, positions[vertices[i].positionIndex].v, sizeof(float) * 3);
		memcpy(vertexes[i].texcoord, texcoords[vertices[i].texcoordIndex].v, sizeof(float) * 2);
		memcpy(vertexes[i].normal, normals[vertices[i].normalIndex].v, sizeof(float) * 3);
	}
	printf("face count %u\n", objIndexes.size() / 3);
	delete[] fileContent;  // 释放文件缓存
	return vertexes;

}