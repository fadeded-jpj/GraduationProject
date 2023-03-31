#pragma once

#include "Shape.h"
#include "BVH.h"

#include <vector>


class Scene {
private:
	std::vector<shape*> models;
	std::vector<Light> Lights;

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	std::vector<Triangle_encoded> trianglesData;
	std::vector<BVHNode_encode> BVHData;
	
	// 三角形数据
	GLuint tbo, texBuffer;
	
	// BVH数据
	GLuint bvh_tbo, bvh_texBuffer;

	GLuint VAO, VBO, EBO;
	bool inited, modified;
public:
	Scene();
	Scene(std::vector<shape*> model);
	~Scene();

	void push(shape* s);
	void push(Light light);
	
	void Render(Shader& shader);
	void Draw(Shader& shader);

	float HitAABB(Ray ray);
	HitResult HitArray(Ray ray, int l, int r);
	HitResult HitTriangle(Triangle_encoded t, Ray ray);
	HitResult HitBVH(Ray ray);

private:
	void initTrianglesData();
	void initBVHData();

	template<class T>
	void myBindBuffer(GLuint& tbo, GLuint& buffer, std::vector<T>& data, GLuint idx = 0);
};

template<class T>
inline void Scene::myBindBuffer(GLuint& tbo, GLuint& buffer, std::vector<T>& data, GLuint idx)
{
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, data.size() * sizeof(T), &data[0], GL_STATIC_DRAW);

	glGenTextures(1, &buffer);
	glActiveTexture(GL_TEXTURE0 + idx);
	glBindTexture(GL_TEXTURE_BUFFER, buffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);
}
