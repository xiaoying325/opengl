#pragma once

struct VertexData
{
	float position[3];
	float texcoord[2];
	float normal[3];
};


//����һ������ģ�͵ĺ���
VertexData* LoadObjModel(const char* filePath, unsigned int** indexes, int& vertexCount, int& indexCount);