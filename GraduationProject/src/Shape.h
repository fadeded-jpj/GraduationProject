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
};

struct Material
{
	glm::vec3 emissive = glm::vec3(0, 0, 0);			// �Ƿ񷢹�
	glm::vec3 normal = glm::vec3(0, 0, 0);				// ������
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);		// ��ɫ
	float subsurface = 0.0f;
	float roughness = 0.1f;
	float metallic = 0.0;

	float specular = 0.3f;
	float specularTint = 0.5f;
	float anisotropic = 0.0f;

	float sheen = 0.0f;
	float sheenTine = 0.0f;
	float clearcoat = 0.1f;
	
	float clearcoatGloss = 0.1f;
	
	Material(glm::vec3 color = glm::vec3(0),float roughness = 0.9f,float metallic = 0.0f, float specular = 0.3f)
		:color(color), roughness(roughness), metallic(metallic), specular(specular) {}
};

struct HitResult
{
	bool isHit = false;							// �Ƿ�����
	double distance = 0.0f;						// �뽻��ľ���
	glm::vec3 hitPoint = glm::vec3(0, 0, 0);	// �������е�
	Material material;							// ���е�ı������

	glm::vec3 pos;                              //���е�λ��
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

	// �洢��������Ƭ����
	std::vector<Triangle_encoded> triangles; 

public:
	glm::vec3 center;
	float R;
	Material material;

private:
	void encodedData();

public:
	Sphere(const glm::vec3 center = { 0.0f, 0.0f, 0.0f }, const float R = 0.0f, const glm::vec3 color = { 0.0f, 0.0f, 0.0f });
	Sphere( const glm::vec3 center, const float R, Material material);
	~Sphere();
	
	void Draw(Shader& shader);
	HitResult intersect(Ray ray) { return HitResult(); }
	void setEmissive(glm::vec3 emissive);

	inline std::vector<Triangle_encoded> getCodedData() { return triangles; }
};

class Plane : public shape
{
private:
	std::vector<Triangle_encoded> triangles;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	unsigned int VAO, VBO, EBO;

public:
	std::vector<glm::vec3> points;
	glm::vec3 normal;
	Material materal;

private:
	void encodeData();
public:
	Plane() {}
	Plane(std::vector<glm::vec3> points, glm::vec3 normal, Material material);
	~Plane();

	void Draw(Shader& shader);
	HitResult intersect(Ray ray) { return HitResult(); }
	void setEmissive(glm::vec3 emissive) {}

	inline std::vector<Triangle_encoded> getCodedData() { return triangles; };
};

class Cube : public shape
{
private:
	glm::vec3 vertices;	//����
	glm::vec3 center;

	std::vector<std::vector<glm::vec3>> planeVertices;
	std::vector<glm::vec3> planeNormal;

	std::vector<Triangle_encoded> triangles;

public:
	Material material;

	float Length, Width, Height;

	std::vector<Plane> planes;

private:
public:
	Cube(glm::vec3 center, Material material, float X = 0.5f, float Y = 0.5f, float Z = 0.5f, float rotateY = 0.0f);
	~Cube() {}

	void Draw(Shader& shader);
	HitResult intersect(Ray ray) { return HitResult(); }
	void setEmissive(glm::vec3 emissive) {}
	void changeMaterial(Material m);
	
	void encodeData();

	inline std::vector<Triangle_encoded> getCodedData() { return triangles; };
};