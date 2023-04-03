#pragma once
#ifndef __SETTING_H__
#define __SETTING_H__

#include "Camera.h"
#include "Shape.h"
#include <random>
#include <vector>


//-------- windows settings-----------
extern const unsigned int SCR_WIDTH = 1200;
extern const unsigned int SCR_HEIGHT = 1200;


//----------camera settings------------
extern Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));
float lastX = SCR_HEIGHT / 2.0f;
float lastY = SCR_WIDTH / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int frameCount = 0;

//-----------end------------------------

const float PI = 3.14159265359;

//----------Connle Box DATA-----------------
std::vector<glm::vec3> Left = {
	{-2,-2,-2},
	{-2, 2,-2},
	{-2, 2,-7},
	{-2,-2,-7}
};
std::vector<glm::vec3> Right = {
	{ 2,-2,-2},
	{ 2, 2,-2},
	{ 2, 2,-7},
	{ 2,-2,-7}
};

std::vector<glm::vec3> Up = {
	{ 2, 2,-2},
	{-2, 2,-2},
	{-2, 2,-7},
	{ 2, 2,-7}
};

std::vector<glm::vec3> Down = {
	{ 2,-2,-2},
	{-2,-2,-2},
	{-2,-2,-7},
	{ 2,-2,-7}
};

std::vector<glm::vec3> Back = {
	{-2,-2,-7},
	{-2, 2,-7},
	{ 2, 2,-7},
	{ 2,-2,-7}
};

std::vector<glm::vec3> LightUp = {
	{ 0.5f, 1.99f,-3},
	{-0.5f, 1.99f,-3},
	{-0.5f, 1.99f,-6},
	{ 0.5f, 1.99f,-6}
};

Material RED({ 1,0,0 });
Material GREEN({ 0,1,0 });  
Material BLUE({ 0,0,1 });   
Material YELLOW({ 1,1,0 }); 
Material PURPLE({ 1,0,1 }); 
Material CYAN({ 0,1,1 });   
Material WHITE({ 1,1,1 });  
Material GREY({ 0.8f,0.8f,0.8f });
Material WHITE_MIRROR({ 1,1,1 }, 0.05f, 0.9f, 0.0f);

#endif // !__SETTING_H__
