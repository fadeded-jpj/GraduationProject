#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Shader.h"
#include "encoded/Triangle.h"


struct Light
{
	glm::vec3 Pos;
	glm::vec3 Color;
};

class Ray
{
public:
	glm::vec3 start;
	glm::vec3 dir;
public:
	Ray(const glm::vec3 start, const glm::vec3 dir) :start(start), dir(glm::normalize(dir)) {}
	Ray(Light light);
};



struct Material
{
	glm::vec3 emissive = glm::vec3(0, 0, 0);			// 是否发光
	glm::vec3 normal = glm::vec3(0, 0, 0);				// 法向量
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);		// 颜色
	float ao = 1.0f;
	float roughness = 0.1f;
	float metallic = 0.0f;
};

struct HitResult
{
	bool isHit = false;							// 是否命中
	double distance = 0.0f;						// 与交点的距离
	glm::vec3 hitPoint = glm::vec3(0, 0, 0);	// 光线命中点
	Material material;							// 命中点的表面材质

	glm::vec3 pos;                              //命中点位置
};

class shape
{
public:
	shape() {}
	virtual HitResult intersect(Ray ray) = 0;
	virtual void Draw(Shader& shader) = 0;
	virtual std::vector<Triangle_encoded> getCodedData() = 0;
	virtual void setEmissive(glm::vec3 emissive) = 0;
};

struct Triangle : public shape
{
public:
	glm::vec3 p1, p2, p3;
	glm::vec3 n1, n2, n3;
	Material material;

private:
	unsigned int va, vb;

public:
	Triangle(const glm::vec3 v0, const glm::vec3 v1, const glm::vec3 v2, const glm::vec3 color = {1.0f, 1.0f, 1.0f});
	~Triangle();
	
	HitResult intersect(Ray ray) { return HitResult(); }
	void Draw(Shader shader) {}

	std::vector<Triangle_encoded> encodeData(std::vector<Triangle>& triangle);
};

class Sphere : public shape
{
private:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uv;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;
	
	std::vector<float> data;	// pos  normal  texCoord
	unsigned int VAO, VBO, EBO;

	// 存储三角形面片数据
	std::vector<Triangle_encoded> triangles; 

public:
	glm::vec3 center;
	float R;
	Material material;

private:
	void encodedData();

public:
	Sphere(const glm::vec3 center = { 0.0f, 0.0f, 0.0f }, const float R = 0.0f, const glm::vec3 color = { 0.0f, 0.0f, 0.0f });
	~Sphere();
	
	void Draw(Shader& shader);
	HitResult intersect(Ray ray) { return HitResult(); }
	void setEmissive(glm::vec3 emissive);

	std::vector<Triangle_encoded> getCodedData() { return triangles; }
};

class Square : public shape
{
private:
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;

	Material materal;
public:
	Square();
	Square(std::vector<glm::vec3> points);
};