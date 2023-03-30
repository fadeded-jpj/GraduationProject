#include "Shape.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <vector>
#include <float.h>
#include <random>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Camera.h"

extern Camera camera;
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

//--------random function------------------
std::uniform_real_distribution<> dis(0.0, 1.0);
std::random_device rd;
std::mt19937 gen(rd());
extern double randf()
{
    return dis(gen);
}

extern glm::vec3 randomVec3()
{
    // 获取光源单位半球面方向向量
    glm::vec3 d = 2.0f * glm::vec3(randf(), randf(), randf()) - glm::vec3(1.0f);
    while (glm::dot(d, d) > 1.0f)
        d = 2.0f * glm::vec3(randf(), randf(), randf()) - glm::vec3(1.0f);

    return d;
}

extern glm::vec3 randomDir(glm::vec3 n)
{
    // 球面反射随机向量
    return glm::normalize(randomVec3() + n);
}

//-----------end------------------------

const float PI = 3.14159265359f;
const unsigned int X_SEGMENTS = 8;
const unsigned int Y_SEGMENTS = 8;


Triangle::Triangle(const glm::vec3 v0, const glm::vec3 v1, const glm::vec3 v2, const glm::vec3 color)
    :p1(v0),p2(v1), p3(v2)
{
    material.normal = glm::normalize(glm::cross(p1 - p2, p1 - p3));
    material.color = color;

    float ver[] = {
        p1.x, p1.y, p1.z,
        p2.x, p2.y, p2.z,
        p3.x, p3.y, p3.z,
    };

    glGenBuffers(1, &vb);
    glGenVertexArrays(1, &va);
    
    glBindVertexArray(va);
    
    glBindBuffer(GL_ARRAY_BUFFER, vb);    
    glBufferData(GL_ARRAY_BUFFER, sizeof(ver), ver, GL_STATIC_DRAW);

    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
}

Triangle::~Triangle()
{
    glDeleteBuffers(1, &vb);
    glDeleteVertexArrays(1, &va);
}


void Sphere::encodedData()
{
    if (!triangles.empty())
        triangles.clear();

    /*
    for (unsigned int y = 0; y < Y_SEGMENTS; y++)
    {
        for (unsigned int x = 0; x < X_SEGMENTS; x++)
        {
            //三角形索引 1 ， 2
            GLuint Pos10 = y * (X_SEGMENTS + 1) + x;
            GLuint Pos11 = (y + 1) * (X_SEGMENTS + 1) + x;
            GLuint Pos12 = (y + 1) * (X_SEGMENTS + 1) + x + 1;

            GLuint Pos20 = y * (X_SEGMENTS + 1) + x;
            GLuint Pos21 = (y + 1) * (X_SEGMENTS + 1) + x + 1;
            GLuint Pos22 = y * (X_SEGMENTS + 1) + x + 1;

            // 储存每一个三角形面片数据
            Triangle_encoded tri1, tri2;
            tri1.p1 = positions[Pos10];
            tri1.p2 = positions[Pos11];
            tri1.p3 = positions[Pos12];
            tri1.n1 = normals[Pos10];
            tri1.n2 = normals[Pos11];
            tri1.n3 = normals[Pos12];

            tri2.p1 = positions[Pos20];
            tri2.p2 = positions[Pos21];
            tri2.p3 = positions[Pos22];
            tri2.n1 = normals[Pos20];
            tri2.n2 = normals[Pos21];
            tri2.n3 = normals[Pos22];
            
            tri1.emissive = material.emissive;
            tri1.baseColor = material.color;
            tri1.param1 = glm::vec3(material.ao, material.roughness, material.metallic);

            tri2.emissive = material.emissive;
            tri2.baseColor = material.color;
            tri2.param1 = glm::vec3(material.ao, material.roughness, material.metallic);

            triangles.push_back(tri1);
            triangles.push_back(tri2);
        }
    }
    */

    for (int i = 2; i < indices.size(); i++)
    {
        GLuint Pos0;
        GLuint Pos1;
        GLuint Pos2;
        if (i % 2)  // 奇数
        {
            Pos0 = indices[i - 2];
            Pos1 = indices[i - 1];
            Pos2 = indices[i];
        }
        else
        {
            Pos0 = indices[i - 1];
            Pos1 = indices[i - 2];
            Pos2 = indices[i];
        }
            
        Triangle_encoded t;
        t.p1 = positions[Pos0];
        t.p2 = positions[Pos1];
        t.p3 = positions[Pos2];

        t.n1 = normals[Pos0];
        t.n2 = normals[Pos1];
        t.n3 = normals[Pos2];

        t.emissive = material.emissive;
        t.baseColor = material.color;
        t.param1 = glm::vec3(material.ao, material.roughness, material.metallic);

        triangles.push_back(t);       
    }

}

Sphere::Sphere(const glm::vec3 center, const float R, const glm::vec3 color)
    :center(center), R(R)
{
    material.color = color;

    glGenVertexArrays(1, & VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // 画球

    /*
    for (unsigned int x = 0; x <= X_SEGMENTS; x++)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; y++)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions.push_back(R * glm::vec3(xPos, yPos, zPos) + center);
            //positions.push_back(glm::vec3(xPos, yPos, zPos));

            std::cout << "(" << xPos << "," << yPos << "," << zPos << ")" << std::endl;

            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }
    
    //bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        for (int x = 0; x < X_SEGMENTS; x++)
        {   
            //三角形索引 1 ， 2
            GLuint Pos10 = y * (X_SEGMENTS + 1) + x;
            GLuint Pos11 = (y + 1) * (X_SEGMENTS + 1) + x;
            GLuint Pos12 = (y + 1) * (X_SEGMENTS + 1) + x + 1;

            GLuint Pos20 = y * (X_SEGMENTS + 1) + x;
            GLuint Pos21 = (y + 1) * (X_SEGMENTS + 1) + x + 1;
            GLuint Pos22 = y * (X_SEGMENTS + 1) + x + 1;
            
            indices.push_back(Pos10);
            indices.push_back(Pos11);
            indices.push_back(Pos12);
            
            indices.push_back(Pos20);
            indices.push_back(Pos21);
            indices.push_back(Pos22);
        }
    }
    unsigned int indexCnt = static_cast<unsigned int>(indices.size());

    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
        if (uv.size() > 0)
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
    }
    */
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions.push_back(R * glm::vec3(xPos, yPos, zPos) + center);
            //positions.push_back(glm::vec3(xPos, yPos, zPos));
            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    unsigned int indexCount = static_cast<unsigned int>(indices.size());

    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }

        if (uv.size() > 0)
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
    }
    // 三角形数据传入
    encodedData();

    // 设置VAO，EBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);
    unsigned int stride = (3 + 3 + 2) * sizeof(float);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
}

Sphere::~Sphere()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}


void Sphere::setEmissive(glm::vec3 emissive)
{
    material.emissive = emissive;
    encodedData();
}


std::vector<Triangle_encoded> Triangle::encodeData(std::vector<Triangle>& triangles)
{
    int n = triangles.size();
    std::vector<Triangle_encoded> res(n);

    for (int i = 0; i < n; i++)
    {
        Triangle& t = triangles[i];
        Material& m = t.material;

        // vertex position
        res[i].p1 = t.p1;
        res[i].p2 = t.p2;
        res[i].p3 = t.p3;

        // normal
        res[i].n1 = t.n1;
        res[i].n2 = t.n2;
        res[i].n3 = t.n3;

        // material
        res[i].emissive = m.emissive;
        res[i].baseColor = m.color;
        res[i].param1 = glm::vec3(m.ao, m.roughness, m.metallic);
    }

    return res;
}

void Sphere::Draw(Shader& shader) 
{
    unsigned int indexCount = static_cast<unsigned int>(indices.size());
    shader.Bind();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}