#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <vector>

#include "glm/glm.hpp"

//#include "../Shape.h"

struct Material_encoded {
    glm::vec3 emissive = glm::vec3(0, 0, 0);  // 作为光源时的发光颜色
    glm::vec3 baseColor = glm::vec3(1, 1, 1);

    float ao = 1.0f;
    float roughness = 0.1f;
    float metallic = 0.0f;
};

struct Triangle_encoded {
    glm::vec3 p1, p2, p3;
    glm::vec3 n1, n2, n3;
    glm::vec3 emissive;      // 自发光参数
    glm::vec3 baseColor;     // 颜色
    glm::vec3 param1;        // ao roughness metallic

    bool operator == (Triangle_encoded& t);
};

void transmitToBuffer(GLuint& tbo, GLuint& TexBuffer, std::vector<Triangle_encoded> data);