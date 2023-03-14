#pragma once

#include "Shape.h"

#include <vector>


class Scene {
private:
	std::vector<shape*> models;
	std::vector<Light> Lights;

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	std::vector<Triangle_encoded> trianglesData;
	GLuint tbo, texBuffer;
	GLuint VAO, VBO, EBO;
	bool inited, modified;
public:
	Scene();
	Scene(std::vector<shape*> model);
	~Scene();

	void push(shape* s);
	void push(Light light);
	
	void Render(Shader& shader);
private:
	void initTrianglesData();
};