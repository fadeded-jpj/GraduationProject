#include "TestPathTracingPhoto.h"

#include "imgui/imgui.h"

#include "../Camera.h"

extern newCamera newcamera;

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
	m_PathTracingShader->SetUniform1i("width", SCR_WIDTH );
	m_PathTracingShader->SetUniform1i("height", SCR_HEIGHT );

	m_FrameShader = std::make_unique<Shader>("res/shaders/lastframe.shader");
	m_FrameShader->Bind();
	m_FrameShader->SetUniform1i("texture0", 0);

	m_PBRShader = std::make_unique<Shader>("res/shaders/rander.shader");
	m_PBRShader->Bind();
	m_PBRShader->SetUniform3fv("light.pos", lightPos);
	m_PBRShader->SetUniform3fv("light.color", LightMaterial.emissive);

	m_LightShader = std::make_unique<Shader>("res/shaders/basic.shader");

	m_DenoiseShader = std::make_unique<Shader>("res/shaders/denoise.shader");
	m_DenoiseShader->Bind();
	m_DenoiseShader->SetUniform1i("texture0", 0);

	// 物体
	// plane
	planes.push_back(std::make_unique<Plane>(Left, glm::vec3(-1, 0, 0), RED));
	planes.push_back(std::make_unique<Plane>(Right, glm::vec3(-1, 0, 0), GREEN));
	planes.push_back(std::make_unique<Plane>(Up, glm::vec3(0, -1, 0), WHITE));
	planes.push_back(std::make_unique<Plane>(Down, glm::vec3(0, -1, 0), WHITE));
	planes.push_back(std::make_unique<Plane>(Back, glm::vec3(0, 0, -1), WHITE_MIRROR));
	planes.push_back(std::make_unique<Plane>(LightUp, glm::vec3(0, -1, 0), LightMaterial));

	// cube
	cubes.push_back(std::make_unique<Cube>(glm::vec3(0.3, -1.5, -6.0), cube1Material));
	cubes.push_back(std::make_unique<Cube>(glm::vec3(-1.0, -1.2, -4.1), cube2Material, 0.5, 0.8, 0.5, PI / 4.0f));

	WHITE.roughness = 0.3f;

	// sphere
	spheres.push_back(std::make_unique<Sphere>(glm::vec3(-1.0, 0, -4.1), 0.5, WHITE_MIRROR));

	// model
	models.push_back(std::make_unique<Model>(modelPath));
	models[0]->encodedData(modelPos);

	// 场景
	m_Scene = std::make_unique<Scene>();
	
	initScene();
	
	// 帧缓冲区
	m_FBO = std::make_unique<FrameBuffer>();
	m_FBO_OUT = std::make_unique<FrameBuffer>();
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

	if (!generate)
	{
		frameCounter = 0;
		//TODO: render by phong

		updateView();
	}
	else
	{
		if (bufferClear)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			frameCounter = 0;
		}

		m_FBO->Bind();
		if (bufferClear)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			bufferClear = false;
		}

		m_PathTracingShader->Bind();
		m_PathTracingShader->SetUniform1i("frameCount", frameCounter++);
		m_PathTracingShader->SetUniform3fv("camera.lower_left_corner", lower_left_corner);
		m_PathTracingShader->SetUniform3fv("camera.horizontal", horizontal);
		m_PathTracingShader->SetUniform3fv("camera.vertical", vertical);
		m_PathTracingShader->SetUniform3fv("camera.origin", origin);

		m_DenoiseShader->Bind();
		m_DenoiseShader->SetUniform1i("frameCout", frameCounter);

		// 渲染数据读入帧缓冲区
		m_FBO->BindTexture(2);
		m_Scene->Render(*m_PathTracingShader);
		m_FBO->DrawBuffer(1);

		// 绘制缓冲区内容到输出缓冲区
		m_FBO->UnBind();

		m_FBO_OUT->Bind();
		m_FBO_OUT->BindTexture(2);

		m_FBO->Draw(*m_FrameShader);

		m_FBO_OUT->DrawBuffer(1);

		//输出缓冲区处理后绘制图像
		m_FBO_OUT->UnBind();
		m_FBO_OUT->Draw(*m_DenoiseShader);
	}
}

void test::TestRenderPhoto::OnImGuiRender()
{	
	ImGui::Checkbox("generate", &generate);

	ImGui::BulletText("cube1");
	ImGui::SliderFloat3("cube 1 color", &cube1Material.color.x, 0.0f, 1.0f);

	ImGui::BulletText("cube 2");
	ImGui::SliderFloat3("cube 2 color", &cube2Material.color.x, 0.0f, 1.0f);

	ImGui::BulletText("sphere");

	ImGui::BulletText("model");
	ImGui::SliderFloat3("model position", &modelPos.x, -6.0, 2.0);
	
	if (ImGui::Button("modify position"))
	{
		bufferClear = true;
		cubes[0]->changeMaterial(cube1Material);
		cubes[1]->changeMaterial(cube2Material);
		models[0]->encodedData(modelPos);
		initScene();
		m_Scene->setInited();
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}


void test::TestRenderPhoto::initScene()
{
	m_Scene->clearData();
	
	for (auto& plane : planes)
		m_Scene->push(plane.get());
	for (auto& cube : cubes)
		m_Scene->push(cube.get());
	for (auto& sphere : spheres)
		m_Scene->push(sphere.get());
	for (auto& model : models)
		m_Scene->push(model.get());
}

void test::TestRenderPhoto::updateView()
{
	lower_left_corner = newcamera.lower_left_corner;
	horizontal = newcamera.horizontal;
	vertical = newcamera.vertical;
	origin = newcamera.origin;
}