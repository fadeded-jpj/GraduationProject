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
extern float lastX = SCR_HEIGHT / 2.0f;
extern float lastY = SCR_WIDTH / 2.0f;
extern bool firstMouse = true;
extern float deltaTime = 0.0f;
extern float lastFrame = 0.0f;

unsigned int frameCount = 0;

//-----------end------------------------

const float PI = 3.14159265359;

//----------Connle Box DATA-----------------
extern std::vector<glm::vec3> Left = {
	{-2,-2,-3},
	{-2, 2,-3},
	{-2, 2,-8},
	{-2,-2,-8}
};
extern std::vector<glm::vec3> Right = {
	{ 2,-2,-3},
	{ 2, 2,-3},
	{ 2, 2,-8},
	{ 2,-2,-8}
};

extern std::vector<glm::vec3> Up = {
	{ 2, 2,-3},
	{-2, 2,-3},
	{-2, 2,-8},
	{ 2, 2,-8}
};

extern std::vector<glm::vec3> Down = {
	{ 2,-2,-3},
	{-2,-2,-3},
	{-2,-2,-8},
	{ 2,-2,-8}
};

extern std::vector<glm::vec3> Back = {
	{-2,-2,-8},
	{-2, 2,-8},
	{ 2, 2,-8},
	{ 2,-2,-8}
};

extern std::vector<glm::vec3> LightUp = {
	{ 0.5f, 1.999f,-5},
	{-0.5f, 1.999f,-5},
	{-0.5f, 1.999f,-7},
	{ 0.5f, 1.999f,-7}
};

extern Material RED({ 1,0,0 });
extern Material GREEN({ 0,1,0 });  
extern Material BLUE({ 0,0,1 });   
extern Material YELLOW({ 1,1,0 }); 
extern Material PURPLE({ 1,0,1 }); 
extern Material CYAN({ 0,1,1 });   
extern Material WHITE({ 1,1,1 });  
extern Material GREY({ 0.8f,0.8f,0.8f });
extern Material WHITE_MIRROR({ 1,1,1 }, 0.05f, 1.0f, 0.0f);

#endif // !__SETTING_H__
