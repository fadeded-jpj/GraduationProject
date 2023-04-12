#include "TestPathTracingPhoto.h"

#include "imgui/imgui.h"


test::TestRenderPhoto::TestRenderPhoto()
{
	Material LightMaterial({ 1,1,1 });
	LightMaterial.emissive = glm::vec3(1.0f);

	GREEN.roughness = 0.05f;
	GREEN.metallic = 0.3f;

	for (auto& v : LightUp)
	{
		lightPos += v;
	}
	lightPos /= LightUp.size();

	// 着色器
	m_PathTracingShader = std::make_unique<Shader>("res/shaders/pathTracing.shader");
	m_PathTracingShader->Bind();
	m_PathTracingShader->SetUniform1i("lastFrame", 2);
	m_PathTracingShader->SetUniform1i("width", SCR_WIDTH);
	m_PathTracingShader->SetUniform1i("height", SCR_HEIGHT);

	m_FrameShader = std::make_unique<Shader>("res/shaders/lastframe.shader");
	m_FrameShader->Bind();
	m_FrameShader->SetUniform1i("texture0", 0);

	m_PBRShader = std::make_unique<Shader>("res/shaders/rander.shader");
	m_PBRShader->Bind();
	m_PBRShader->SetUniform3fv("light.pos", lightPos);
	m_PBRShader->SetUniform3fv("light.color", LightMaterial.emissive);

	m_LightShader = std::make_unique<Shader>("res/shaders/basic.shader");

	// 物体
	// 盒子
	planes.push_back(std::make_unique<Plane>(Left, glm::vec3(-1, 0, 0), RED));
	planes.push_back(std::make_unique<Plane>(Right, glm::vec3(-1, 0, 0), GREEN));
	planes.push_back(std::make_unique<Plane>(Up, glm::vec3(0, -1, 0), WHITE));
	planes.push_back(std::make_unique<Plane>(Down, glm::vec3(0, -1, 0), WHITE));
	planes.push_back(std::make_unique<Plane>(Back, glm::vec3(0, 0, -1), WHITE_MIRROR));
	planes.push_back(std::make_unique<Plane>(LightUp, glm::vec3(0, -1, 0), LightMaterial));

	// cube
	cubes.push_back(std::make_unique<Cube>(glm::vec3(0.3, -1.5, -7.0), CYAN));
	cubes.push_back(std::make_unique<Cube>(glm::vec3(-1.0, -1.2, -5.1), WHITE, 0.5, 0.8, 0.5, PI / 4.0f));

	WHITE.roughness = 0.3f;

	// sphere
	//spheres.push_back(std::make_unique<Sphere>(glm::vec3(-1.0, 0.5, -5.0), 0.3, WHITE));
	//spheres.push_back(std::make_unique<Sphere>(glm::vec3(1.5, -1, -6), 0.3, WHITE));

	// model
	models.push_back(std::make_unique<Model>(modelPath));

	// 场景
	m_Scene = std::make_unique<Scene>();
	
	for (auto& plane : planes)
		m_Scene->push(plane.get());
	for (auto& cube : cubes)
		m_Scene->push(cube.get());
	for (auto& sphere : spheres)
		m_Scene->push(sphere.get());
	for (auto& model : models)
		m_Scene->push(model.get());
	
	// 帧缓冲区
	m_FBO = std::make_unique<FrameBuffer>();
}

test::TestRenderPhoto::~TestRenderPhoto()
{
}

void test::TestRenderPhoto::OnUpdate(float deltaTime)
{
}

void test::TestRenderPhoto::OnRender()
{
	float currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	//m_LightShader->Bind();
	//planes[0]->Draw(*m_LightShader);

	if (lastMode != currentMode)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		frameCounter = 0;
	}


	m_PathTracingShader->Bind();
	m_PathTracingShader->SetUniform1i("mode", currentMode);

	m_FBO->Bind();
	if (lastMode != currentMode)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	m_PathTracingShader->Bind();
	m_PathTracingShader->SetUniform1i("frameCount", frameCounter++);
	
	// 渲染数据读入帧缓冲区
	m_FBO->BindTexture(2);
	m_Scene->Render(*m_PathTracingShader);
	m_FBO->DrawBuffer(1);
	
	// 绘制缓冲区内容
	m_FBO->UnBind();
	m_FBO->Draw(*m_FrameShader);

	lastMode = currentMode;
}

void test::TestRenderPhoto::OnImGuiRender()
{
	ImGui::SliderInt("spp", &spp, 1, 128);
	
	if (ImGui::Button("Diffuse"))
		currentMode = Mode::Diffuse;
	if (ImGui::Button("Specular"))
		currentMode = Mode::Specular;
	if (ImGui::Button("All"))
		currentMode = Mode::All;

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}
