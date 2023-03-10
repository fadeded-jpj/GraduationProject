#pragma once

#include "Shape.h"

#include <vector>


class Scene {
private:
	std::vector<std::pair<shape*, std::string>> models;
	std::vector<Light> Lights;

	std::vector<Triangle_encoded> trianglesData;
	GLuint tbo, texBuffer;
	bool inited, modified;
public:
	Scene() :inited(false), modified(false), tbo(0), texBuffer(0){}
	Scene(std::vector<std::pair<shape*, std::string>> model);
	~Scene() {}

	void push(shape* s, std::string path);
	void push(Light light);
	
	glm::vec3 pathTracing(Ray ray, int depth);

	void Draw();
private:
	void initTrianglesData();
};