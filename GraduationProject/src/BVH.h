#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "encoded/Triangle.h"

struct BVHNode {
	int left, right;	//左右孩子下标
	int n;
	int index;
	glm::vec3 AA, BB;	// hitbox AA为下限 BB为上限
};

int BuildBVH(std::vector<Triangle_encoded>& triangles, std::vector<BVHNode>& nodes, int left, int right, int n);

struct BVHNode_encode {
	glm::vec3 children;	//(left right __)
	glm::vec3 information;	// n index __
	glm::vec3 AA;
	glm::vec3 BB;
};

BVHNode_encode encodeBVH(const BVHNode& bvh);