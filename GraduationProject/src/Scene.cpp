#include "Scene.h"

#include <random>
#include <iostream>

//--------random function------------------
//extern std::uniform_real_distribution<> dis(0.0, 1.0);
//extern std::random_device rd;
//extern std::mt19937 gen(rd());
extern double randf();
extern glm::vec3 randomVec3();
extern glm::vec3 randomDir(glm::vec3 n);


//-----------end------------------------

Scene::Scene(std::vector<std::pair<shape*, std::string>> model)
	:models(model), inited(false), modified(false), tbo(0), texBuffer(0)
{}

void Scene::push(shape* s, std::string path)
{
	models.push_back({s, path});
}

void Scene::push(Light light)
{
	Lights.push_back(light);
}

glm::vec3 Scene::pathTracing(Ray ray, int depth)
{
	glm::vec3 color = glm::vec3(0);

	//�ݹ����
	if(depth <= 0)
		return color;
	
	// ʵ��RR
	double r = randf();
	float p = 0.8;		// RR���и���

	if (r > p)
		return color;
	
	std::vector<shape*> shapes;
	for (auto& [s, _] : models)
		shapes.push_back(s);
	
	HitResult res = shoot(shapes, ray);

	// �Է���ͺ��Է����
	if (res.material.emissive != glm::vec3(0))
		color = res.material.emissive;
	else 
	{
		Ray newRay(res.hitPoint, randomDir(res.material.normal));
		float cosine = glm::dot(-ray.dir, res.material.normal);
		color = res.material.color * pathTracing(newRay, depth - 1);
	}
	
	return color / p;
}

void Scene::Draw()
{
	//TODO: ���������ݰ���model��˳�����δ���buffer, ����index
	int n = models.size();
	int index = 0;

	if (!inited) {
		initTrianglesData();
		inited = true;
	}

	for (int i = 0; i < n; i++) {
		Shader shader(models[i].second);
		auto& model = models[i].first;

		shader.Bind();
		shader.SetUniform1i("index", index);
		//std::cout << i << std::endl;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, texBuffer);
		shader.SetUniform1i("triangles", 0);

		model->Draw(shader);

		index += 32768;
	}
}

void Scene::initTrianglesData()
{
	int n = models.size();
	for (int i = 0; i < n; i++) {
		auto& it = models[i].first;
		auto data = it->getCodedData();

		trianglesData.insert(trianglesData.end(), data.begin(), data.end());	//��ÿһ�����ݴ������ݿ�
	}

	// �󶨻�����
	transmitToBuffer(tbo, texBuffer, trianglesData);
}
