#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

enum Camera_Movement
{
	FORWARD, BACKWARD, LEFT, RIGHT
};

class Camera {
private:
	glm::vec3 m_Pos;
	glm::vec3 m_Front;
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	glm::vec3 m_WorldUp;

	//euler angle
	float m_Pitch;
	float m_Yaw;

	float m_Speed;
	float m_MouseSensitivity;

	float m_Fov;
	glm::vec3 eye;

	void updateCameraVector();
public:
	Camera(const glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f));
	~Camera() {}

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetCameraRotate();
	glm::vec3 GetEye();
	inline float GetFov() const { return m_Fov; }
	inline glm::vec3 GetPosition() const { return m_Pos; }
	inline glm::vec3 GetFront() const { return m_Front; }
	inline glm::vec3 GetUp() const { return m_Up; }
	inline glm::vec3 GetRight() const { return m_Right; }

	void ProcessKeyboard(const Camera_Movement dir, const float delatTime);
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void ProcessMouseScroll(float xoffset, float yoffset);
};