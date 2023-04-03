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
		initBVHData();
		inited = true;
	}

	shader.Bind();
	shader.SetUniform3fv("camera.lower_left_corner", { -2.0, -2.0, -2.0 });
	shader.SetUniform3fv("camera.horizontal", {4, 0, 0});
	shader.SetUniform3fv("camera.vertical", { 0, 4, 0 });
	shader.SetUniform3fv("camera.origin", {0 , 0 , 4});
	//shader.SetUniform3fv("camera.horizontal", 2.0f * camera.GetRight());
	//shader.SetUniform3fv("camera.vertical", 2.0f * camera.GetUp());
	//shader.SetUniform3fv("camera.origin", camera.GetPosition());


	//==
	shader.SetUniform3fv("eye", camera.GetEye());
	shader.SetUniformMat4f("cameraRotate", camera.GetCameraRotate());
	

	//==
	shader.SetUniform1i("triangles", 0);
	shader.SetUniform1i("bvh", 1);
	//shader.SetUniform1i("lastFrame", 2);

	shader.SetUniform1i("triangleCount", trianglesData.size());
	shader.SetUniform1i("BVHCount", BVHData.size());


	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Scene::Draw(Shader& shader)
{
	if (!inited)
	{
		inited = true;
		initTrianglesData();
		initBVHData();
	}
	shader.SetUniform1i("triangles", 0);
	shader.SetUniform1i("bvh", 1);

	shader.SetUniform1i("triangleCount", trianglesData.size());
	shader.SetUniform1i("BVHCount", BVHData.size());

	shader.SetUniform3fv("camera.horizontal", 2.0f * camera.GetRight());
	shader.SetUniform3fv("camera.vertical", 2.0f * camera.GetUp());
	shader.SetUniform3fv("camera.origin", camera.GetPosition());
	shader.SetUniform3fv("camera.lower_left_corner", { -1.0, -1.0, -1.0 });

	for (auto& m : models)
	{
		m->Draw(shader);
	}
}

void Scene::BindTex(GLuint& tex, GLuint slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, tex);
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

	// 绑定缓冲区
	myBindBuffer(tbo, texBuffer, trianglesData, 0);
}

void Scene::initBVHData()
{
	int n = trianglesData.size();
	BVHNode tNode;
	tNode.left = 255;
	tNode.right = 128;
	tNode.n = 30;
	tNode.AA = { 1,1,0 };
	tNode.BB = { 0,1,0 };

	std::vector<BVHNode> nodes{ tNode };

	// 减少BVH层数会变完整 why？0
	BuildBVH(trianglesData, nodes, 0, n - 1, 8);
	
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

FrameBuffer::FrameBuffer()
{
	vertices = {
		-1.0f,  1.0f,
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	// framebuffer
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// create a color attachment texture
	std::vector<GLuint> textureColorbuffer(6);
	for (int i = 0; i < 6; i++) {
		glGenTextures(1, &textureColorbuffer[i]);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureColorbuffer[i], 0);
		attachment.push_back(GL_COLOR_ATTACHMENT0 + i);
	}
	glDrawBuffers(attachment.size(), &attachment[0]);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now    
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &FBO);
	//glDeleteRenderbuffers(1, &RBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void FrameBuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//glBindRenderbuffer(GL_RENDERBUFFER, RBO);
}

void FrameBuffer::UnBind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


void FrameBuffer::Draw(Shader shader, GLuint& tex)
{
	shader.Bind();

	glBindVertexArray(VAO);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
}
