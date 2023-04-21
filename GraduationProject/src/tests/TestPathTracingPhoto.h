#pragma once

#include "Test.h"
#include "../Scene.h"
#include "../Shape.h"
#include "../Model.h"

#include<memory>

extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

extern float deltaTime;
extern float lastFrame;

extern Material RED;
extern Material GREEN;
extern Material BLUE;
extern Material YELLOW;
extern Material PURPLE;
extern Material CYAN;
extern Material WHITE;
extern Material GREY;
extern Material WHITE_MIRROR;

extern std::vector<glm::vec3> Left;
extern std::vector<glm::vec3> Right;
extern std::vector<glm::vec3> Up;
extern std::vector<glm::vec3> Down;
extern std::vector<glm::vec3> Back;
extern std::vector<glm::vec3> LightUp;

extern const float PI;

namespace test {
	enum Mode {
		Diffuse,
		Specular,
		All
	};

	class TestRenderPhoto : public Test
	{
	public:
		TestRenderPhoto();
		~TestRenderPhoto();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;

	private:
		std::unique_ptr<Shader> m_PathTracingShader;
		std::unique_ptr<Shader> m_FrameShader;
		std::unique_ptr<Shader> m_PBRShader;
		std::unique_ptr<Shader> m_LightShader;
		std::unique_ptr<Shader> m_DenoiseShader;
		
		std::unique_ptr<Scene> m_Scene;
		std::unique_ptr<FrameBuffer> m_FBO;
		std::unique_ptr<FrameBuffer> m_FBO_OUT;

		std::vector<std::unique_ptr<Plane>> planes;
		std::vector<std::unique_ptr<Cube>> cubes;
		std::vector<std::unique_ptr<Sphere>> spheres;
		std::vector<std::unique_ptr<Model>> models;

		glm::vec3 lightPos;
		glm::vec3 modelPos = glm::vec3(0.8, -2, -6);
		Material cube1Material = CYAN;
		Material cube2Material = WHITE;

		int spp = 1;
		int frameCounter = 0;
		bool bufferClear = false;
		std::string modelPath = "res/obj/HuTao/HuTao.pmx";
		
		Mode currentMode = Mode::Diffuse;
		Mode lastMode = currentMode;

	private:
		void checkModify();
		void initScene();
	};
}