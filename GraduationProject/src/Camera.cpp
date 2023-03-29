#include "Camera.h"

Camera::Camera(const glm::vec3 pos, const glm::vec3 worldUp)
	:m_Pos(pos), m_WorldUp(worldUp), m_Speed(2.5f),
	m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	m_Pitch(0.0f), m_Yaw(-90.0f), m_MouseSensitivity(0.01f),
	m_Fov(60.0f)
{
	eye = m_Pos;
	updateCameraVector();
}

void Camera::updateCameraVector()
{
	glm::vec3 front;
	front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	front.y = sin(glm::radians(m_Pitch));
	front.z = cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));

	m_Front = glm::normalize(front);
	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
	m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(m_Pos, m_Pos + m_Front, m_Up);
}

glm::mat4 Camera::GetCameraRotate()
{
	glm::mat4 res = glm::lookAt(GetEye(), glm::vec3(0, 0, 0), m_Up);
	return glm::inverse(res);
	//return res;
}

glm::vec3 Camera::GetEye()
{	
	glm::vec3 eye = glm::vec3(-sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)),
		sin(glm::radians(m_Yaw)),
		cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)));
	//glm::vec3 eye = m_Pos;
	eye.x *= 4.0;
	eye.y *= 4.0;
	eye.z *= 4.0;
	//glm::vec3 res = m_Pos;

	//return res;
	return eye;
}

void Camera::ProcessKeyboard(const Camera_Movement dir, const float delatTime)
{
	float speed = m_Speed * delatTime;
	if (dir == Camera_Movement::FORWARD)
		m_Pos += m_Front * speed;
	if (dir == Camera_Movement::BACKWARD)
		m_Pos -= m_Front * speed;
	if (dir == Camera_Movement::LEFT)
		m_Pos -= m_Right * speed;
	if (dir == Camera_Movement::RIGHT)
		m_Pos += m_Right * speed;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= m_MouseSensitivity;
	yoffset *= m_MouseSensitivity;

	m_Yaw += xoffset;
	m_Pitch += yoffset;

	//¿ØÖÆÊÓ½Ç·¶Î§
	if (constrainPitch)
	{
		if (m_Pitch > 89.0f)
			m_Pitch = 89.0f;
		if (m_Pitch < -89.0f)
			m_Pitch = -89.0f;
	}
	updateCameraVector();
}

void Camera::ProcessMouseScroll(float xoffset, float yoffset)
{
	if (m_Fov >= 1.0f && m_Fov <= 100.0f)
		m_Fov -= yoffset;
	if (m_Fov < 1.0f)
		m_Fov = 1.0f;
	if (m_Fov > 100.0f)
		m_Fov = 100.0f;
}
