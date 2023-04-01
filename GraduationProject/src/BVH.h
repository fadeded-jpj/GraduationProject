#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "encoded/Triangle.h"

struct BVHNode {
	int left, right;	//���Һ����±�
	int n;				// ��ǰ�����θ���
	int index;			// ��������ʼ�±�
	glm::vec3 AA, BB;	// hitbox AAΪ���� BBΪ����
};

int BuildBVH(std::vector<Triangle_encoded>& triangles, std::vector<BVHNode>& nodes, int left, int right, int n);

struct BVHNode_encode {
	glm::vec3 children;	//(left right __)
	glm::vec3 information;	// n index __
	glm::vec3 AA;
	glm::vec3 BB;
};

BVHNode_encode encodeBVH(const BVHNode& bvh);