#pragma once

struct VertexData
{
	float position[3];
	float texcoord[2];
	float normal[3];
};


//定义一个加载模型的函数
VertexData* LoadObjModel(const char* filePath, unsigned int** indexes, int& vertexCount, int& indexCount);