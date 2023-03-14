#pragma once
#ifndef __SETTING_H__
#define __SETTING_H__

#include "Camera.h"
#include <random>

//-------- windows settings-----------
extern const unsigned int SCR_WIDTH = 1200;
extern const unsigned int SCR_HEIGHT = 1200;


//----------camera settings------------
extern Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));
extern Camera viewCamera(glm::vec3(0.0f, 0.0f, 4.0f));
float lastX = SCR_HEIGHT / 2.0f;
float lastY = SCR_WIDTH / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int frameCount = 0;

//--------data settings-----------
const float PI = 3.141592653;
const unsigned int X_SEGMENTS = 64;
const unsigned int Y_SEGMENTS = 64;



//-----------end------------------------


#endif // !__SETTING_H__
