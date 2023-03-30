#include "Scene.h"

#include <random>
#include <iostream>

#include "Camera.h"
//--------random function------------------
//extern std::uniform_real_distribution<> dis(0.0, 1.0);
//extern std::random_device rd;
//extern std::mt19937 gen(rd());
extern double randf();
extern glm::vec3 randomVec3();
extern glm::vec3 randomDir(glm::vec3 n);

extern Camera camera;
//-----------end------------------------

Scene::Scene()
	:inited(false), modified(false), tbo(0), texBuffer(0), bvh_tbo(0), bvh_texBuffer(0)
{
	vertices = {
		1.0f,  1.0f, 0.0f,  // top right
		1.0f, -1.0f, 0.0f,  // bottom right
	   -1.0f, -1.0f, 0.0f,  // bottom left
	   -1.0f,  1.0f, 0.0f   // top left 
	};

	indices = {
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

Scene::Scene(std::vector<shape*> model)
	:inited(false), modified(false), tbo(0), texBuffer(0), bvh_tbo(0), bvh_texBuffer(0)
{
	vertices = {
		1.0f,  1.0f, 0.0f,  // top right
		1.0f, -1.0f, 0.0f,  // bottom right
	   -1.0f, -1.0f, 0.0f,  // bottom left
	   -1.0f,  1.0f, 0.0f   // top left 
	};

	indices = {
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

Scene::~Scene()
{
	glDeleteBuffers(1, &tbo);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &bvh_tbo);
	glDeleteVertexArrays(1, &VAO);
	glDeleteTextures(1, &texBuffer);
	glDeleteTextures(1, &bvh_texBuffer);
}

void Scene::push(shape* s)
{
	models.push_back(s);
}

void Scene::push(Light light)
{
	Lights.push_back(light);
}


void Scene::Render(Shader& shader)
{
	if (!inited) {
		initTrianglesData();

		//std::vector<Triangle_encoded> t(20);
		//for (int i = 0; i < 20; i++) {
		//	t[i].p2 = glm::vec3(-1 + i/3.0f, -1 + i/4.0f, -5);
		//	t[i].p1 = glm::vec3(-1 + i/3.0f, -2 + i/4.0f, -5);
		//	t[i].p3 = glm::vec3(-2 + i/3.0f, -1 + i/4.0f, -5);

		//	t[i].n1 = glm::vec3(0, 0, 1);
		//	t[i].n2 = glm::vec3(0, 0, 1);
		//	t[i].n3 = glm::vec3(0, 0, 1);

		//	t[i].baseColor = glm::vec3(1, 1, 1);
		//	t[i].emissive = glm::vec3(1, 1, 1);
		//	t[i].param1 = glm::vec3(0.4, 0.5, 1);

		//	//trianglesData.clear();
		//	trianglesData.push_back(t[i]);
		//}
		//myBindBuffer(tbo, texBuffer, trianglesData, 0);

		initBVHData();
		inited = true;
	}

	shader.Bind();
	shader.SetUniform3fv("camera.lower_left_corner", { -1.0, -1.0, -1.0 });
	shader.SetUniform3fv("camera.horizontal", {2, 0, 0});
	shader.SetUniform3fv("camera.vertical", { 0, 2, 0 });
	shader.SetUniform3fv("camera.origin", {0 , 0 , 0});
	//shader.SetUniform3fv("camera.horizontal", 2.0f * camera.GetRight());
	//shader.SetUniform3fv("camera.vertical", 2.0f * camera.GetUp());
	//shader.SetUniform3fv("camera.origin", camera.GetPosition());


	//==
	shader.SetUniform3fv("eye", camera.GetEye());
	shader.SetUniformMat4f("cameraRotate", camera.GetCameraRotate());
	

	//==
	shader.SetUniform1i("triangles", 0);
	shader.SetUniform1i("bvh", 1);

	shader.SetUniform1i("triangleCount", trianglesData.size());
	shader.SetUniform1i("BVHCount", BVHData.size());

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
}

float Scene::HitAABB(Ray ray)
{
	return 0;
}

HitResult Scene::HitArray(Ray ray, int l, int r)
{
	return HitResult();
}

HitResult Scene::HitTriangle(Triangle_encoded t, Ray ray)
{
	return HitResult();
}

HitResult Scene::HitBVH(Ray ray)
{
	return HitResult();
}


void Scene::initTrianglesData()
{
	int n = models.size();
	for (int i = 0; i < n; i++) {
		auto& it = models[i];
		auto data = it->getCodedData();

		trianglesData.insert(trianglesData.end(), data.begin(), data.end());	//把每一个数据存入数据库
	}
	std::cout << "三角形共" << trianglesData.size() << "个" << std::endl;

	/*std::sort(trianglesData.begin(), trianglesData.end(), [](Triangle_encoded t1, Triangle_encoded t2) {
		glm::vec3 c1 = GetTriangleCenter(t1);
		glm::vec3 c2 = GetTriangleCenter(t2);

		return (c1.x + c1.y + c1.z) < (c2.x + c2.y + c2.z);

		});*/

	// 绑定缓冲区
	//transmitToBuffer(tbo, texBuffer, trianglesData);
	myBindBuffer(tbo, texBuffer, trianglesData, 0);
}

void Scene::initBVHData()
{
	int n = trianglesData.size();
	BVHNode tNode;
	tNode.left = 255;
	tNode.right = 128;
	tNode.n = 0;
	tNode.AA = { 1,1,0 };
	tNode.BB = { 0,1,0 };

	std::vector<BVHNode> nodes{ tNode };

	BuildBVH(trianglesData, nodes, 0, n - 1, 2);
	std::cout << "BVH共" << nodes.size() << "个节点" << std::endl;

	int nNode = nodes.size();
	if (!BVHData.empty())
		BVHData.clear();
	for (int i = 0; i < nNode; i++)
	{
		//std::cout << i << std::endl;
		BVHData.push_back(encodeBVH(nodes[i]));
	}

	// 绑定缓冲区
	myBindBuffer(bvh_tbo, bvh_texBuffer, BVHData, 1);
}
