#include "model.h"
#include "misc.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>


VertexData* LoadObjModel(const char* filePath, unsigned int** indexes, int& vertexCount, int& indexCount) {
	// ��ڿε��ص���ǽӴ����ģ�͵����ݣ����շ��صĽṹ��������Լ���õĽṹ
	// ��һ�� ����ȡģ���ļ�
	char* fileContent = LoadFileContent(filePath);

	if (!fileContent) {
		printf("Failed Load Obj File:%s\n", filePath);
		return nullptr;
	}


	printf("Obj Load Success\n%s\n", fileContent);


	// �ڶ���������Ҫ������
	struct  VertexInfo
	{
		float v[3];
	};


	//����һ���洢������������Ľṹ�壬������ɶ�أ�
	//������ʶÿ�����ÿ������ʹ������Щ���ݣ�������֪��pos��texcoord normal
	// v/t/n ----> postionIndex /texcoordIndex /normalIndex
	struct  VertextDefine
	{
		int positionIndex;
		int texcoordIndex;
		int normalIndex;
	};


	std::vector<VertexInfo> positions;//����Ҫ��������������
	std::vector<VertexInfo> texcoords;//����Ҫ������������������
	std::vector<VertexInfo> normals;//����Ҫ�����ķ�����������


	// �������Ķ�������
	std::vector<VertextDefine> vertices;
	std::vector<unsigned int >objIndexes;// IBO

	// ��ô�⣿������stringstream�����ļ�,ʹ��stringsre������һ���ַ�����ssObjFile����fileContent������Ҳ����Obj�����ݣ�ȫ�����˽�ȥ������
	// ���������Ǻ�����������
	std::stringstream ssObjFile(fileContent);
	std::string line;// ÿһ�е�����
	std::string temp; // temo���԰���������������Ҫ������


	while (std::getline(ssObjFile,line))
	{
		if (line.empty() || line[0] == '#') {
			continue;
		}
		std::stringstream ssOneLine(line);
		char type = line[0];
		if (type == 'v') {

			// v�����ַ�Ϊv vt vn �������������ٽ��н���
			if (line.size() > 1 && line[1] == 't') {
				// �������� ����ֻ��Ҫ��ȡ2��������Ϊ��������Ҳ��UV���� ֻ��������
				ssOneLine >> temp;  // vt 0.000000 0.000000  ���ǰ���������г�һ�ζΣ�>��temp��ʶ ����u���������У��ó���һ���� ���temp
				// Ϊʲô�أ���Ϊ��ônOBj�ļ�ÿһ�п�ͷ���йؼ��ʣ��ȶ���������ô��֮������һ����ʲô����
				// ���ǵ�������Ҫ��Ϊ����������ʣ���ȡ���������

				// vt 0.000000 0.000000
				VertexInfo vi{};
				ssOneLine >> vi.v[0] >> vi.v[1]; // 0.000000 0.000000
				texcoords.push_back(vi);
				printf("[vt] %f,%f\n", vi.v[0], vi.v[1]);

			}
			else if (line.size() > 1 && line[1] == 'n') {
				//������Ϣ
				// vn: ����
				ssOneLine >> temp;
				VertexInfo vi{};
				ssOneLine >> vi.v[0] >> vi.v[1] >> vi.v[2]; // 
				normals.push_back(vi);
				printf("[vn] %f, %f, %f\n", vi.v[0], vi.v[1], vi.v[2]);
			}
			else {
				//������Ϣ
				 // v: ��������
				ssOneLine >> temp;
				VertexInfo vi{};
				ssOneLine >> vi.v[0] >> vi.v[1] >> vi.v[2];
				positions.push_back(vi);
				printf("[v] %f, %f, %f\n", vi.v[0], vi.v[1], vi.v[2]);
			}
		}
		else if (type == 'f') {
			//TODO ������Ϣ
			// �����ݣ�Ŀǰֻ�����
			printf("[f] %s\n", line.c_str());
			// TODO: ���� f -> ��������
			// ��������ֻ����������
			// ��һ�� ȥ��f
			ssOneLine >> temp; //f 1/1/1 2/2/2 3/3/3  ---> 1/1/1 2/2/2 3/3/3
			std::string vertexStr;

			for (int i = 0; i < 3; i++) {
				ssOneLine >> vertexStr;
				size_t pos = vertexStr.find_first_of("/");// 1
				std::string postionIndexStr = vertexStr.substr(0, pos);// ��ȡ���յ㲻����pos

				// �ҵ��ڶ���/������λ��
				size_t pos2 = vertexStr.find_first_of("/", pos + 1);
				std::string texcoordIndexStr = vertexStr.substr(pos + 1, pos2 - pos - 1); //pos2 ������=3��λ�ã� ��ȡ�ĳ�����1
				std::string normalIndexStr = vertexStr.substr(pos2 + 1, vertexStr.length() - pos2 - 1);

				//�Ѷ�������ݴ�obj�ı��н������������������и����⣬�����������������index��Ҫת����
				// ����ΪʲôҪת�����Ҹ�һֱҲ��������
				// �������飬postions��0�� positons��1�� positons[2]
				// Obj�ļ��� f 1/2/3  --->��ʵ����ʵ��ǣ�postions[0] texcoord[1] normal[2]
				VertextDefine vd;
				vd.positionIndex = atoi(postionIndexStr.c_str()) - 1; //atoi ת��һ��int���� ԭ�����ַ���1��ת��int����1 -1��
				vd.texcoordIndex = atoi(texcoordIndexStr.c_str()) - 1;
				vd.normalIndex = atoi(normalIndexStr.c_str()) - 1;

				//�ж������Ƕ����������Ƿ��Ѿ����������ǽ���������������㣬�����ã����ܴ洢���ظ��Ķ��㣬����ظ�����ô��ͷ���
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
					����0 --����0��0��0��
					����1--�� ��1��1��1��

					��2��2��2��

				*/
				if (nCurrentIndex == -1) {
					nCurrentIndex = vertices.size(); //2
					vertices.push_back(vd);
				}

				//���԰Ѷ�����������뵽IBO��
				/*
				f 1/1/1 2/2/2 3/3/3
				f 3/3/3 2/2/2 4/4/4
				6�����㣬�������������ض��ģ�Ϊʲô֮���Զ�����������������������iΪ���Ż�
				
				std::vector<VertextDefine> vertices;//VBO
				std::vector<unsigned int >objIndexes;// IBO

				vertextes���澭����������ȥ���߼�֮��Ķ�������Ϊ
				���� 0 -> (1,1,1)
				���� 1 -> (2,2,2)
				���� 2 -> (3,3,3)
				���� 3 -> (4,4,4)

				�ٿ���IBO������
				objIndexes = [0,1,2,2,1,3] //���ɵ�������������


				*/
				objIndexes.push_back(nCurrentIndex);

			}
		}
	}



	//������IBO��VBO
	indexCount = (int)objIndexes.size();
	*indexes = new unsigned int[indexCount];
	for (int i = 0; i < indexCount; i++) {
		(*indexes)[i] = objIndexes[i];
	}

	//����VBO
	vertexCount = (int)vertices.size();
	VertexData* vertexes = new VertexData[vertexCount];
	for (int i = 0; i < vertexCount; ++i) {
		memcpy(vertexes[i].position, positions[vertices[i].positionIndex].v, sizeof(float) * 3);
		memcpy(vertexes[i].texcoord, texcoords[vertices[i].texcoordIndex].v, sizeof(float) * 2);
		memcpy(vertexes[i].normal, normals[vertices[i].normalIndex].v, sizeof(float) * 3);
	}
	printf("face count %u\n", objIndexes.size() / 3);
	delete[] fileContent;  // �ͷ��ļ�����
	return vertexes;

}