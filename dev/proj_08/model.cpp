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

	std::vector<VertexInfo> positions;//����Ҫ��������������
	std::vector<VertexInfo> texcoords;//����Ҫ������������������
	std::vector<VertexInfo> normals;//����Ҫ�����ķ�����������

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
		}


	}

	delete[] fileContent;  // �ͷ��ļ�����
	return nullptr;

}