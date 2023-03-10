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
const unsigned int X_SEGMENTS = 128;
const unsigned int Y_SEGMENTS = 128;


Triangle::Triangle(const glm::vec3 v0, const glm::vec3 v1, const glm::vec3 v2, const glm::vec3 color)
    :point0(v0),point1(v1), point2(v2)
{
    material.normal = glm::normalize(glm::cross(point0 - point1, point0 - point2));
    material.color = color;

    float ver[] = {
        point0.x, point0.y, point0.z,
        point1.x, point1.y, point1.z,
        point2.x, point2.y, point2.z,
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

HitResult Triangle::intersect(Ray ray)
{
    HitResult res;

    // 光线 O + tD
    glm::vec3 O = ray.start;
    glm::vec3 D = ray.dir;
    
    glm::vec3 N = material.normal;
    if (glm::dot(N, D) < 0.00001f) N = -N;
    
    // 平行与三角形平面
    if (fabs(glm::dot(N, D)) < 0.0001f) return res;

    float t = (glm::dot(N, point0) - glm::dot(N, O)) / glm::dot(N, D);
    
    // 交点在光源后
    if (t < 0.0001f) return res;

    // 交点
    glm::vec3 P = O + t * D;
    double distance = glm::distance(O, P);
    
    res.isHit = true;
    res.distance = distance;
    res.hitPoint = P;
    res.material.normal = N;
    res.pos = O + t * D;

    return res;
}

void Triangle::Draw(Shader shader)
{
    shader.Bind();
    shader.SetUniform3fv("color", material.color);
    glBindVertexArray(va);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}


// 返回最近的交点
HitResult shoot(std::vector<shape*>& shapes, Ray ray)
{
    HitResult res, tmp;
    res.distance = DBL_MAX;

    for (auto& shape : shapes)
    {
        tmp = shape->intersect(ray);
        if (tmp.isHit && tmp.distance < res.distance)
            res = tmp;
    }
    
    return res;
}

void Sphere::encodedData()
{
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

            tri1.baseColor = material.color;
            tri1.emissive = material.emissive;
            tri1.param1 = glm::vec3(material.ao, material.roughness, material.metallic);

            tri2.baseColor = material.color;
            tri2.emissive = material.emissive;
            tri2.param1 = glm::vec3(material.ao, material.roughness, material.metallic);

            triangles.push_back(tri1);
            triangles.push_back(tri2);
        }
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
    for (unsigned int x = 0; x <= X_SEGMENTS; x++)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; y++)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions.push_back(glm::vec3(xPos, yPos, zPos));
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

        //if (!oddRow) // even rows: y == 0, y == 2; and so on
        //{
        //    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        //    {
        //        indices.push_back(y * (X_SEGMENTS + 1) + x);
        //        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        //    }
        //}
        //else
        //{
        //    for (int x = X_SEGMENTS; x >= 0; --x)
        //    {
        //        indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        //        indices.push_back(y * (X_SEGMENTS + 1) + x);
        //    }
        //}
        //oddRow = !oddRow;
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

void Sphere::Draw(Shader& shader)
{
    shader.Bind();
    glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0);
    model = glm::translate(model, center);
    model = glm::scale(model, { R,R,R });

    shader.SetUniformMat4f("projection", projection);
    shader.SetUniformMat4f("view", view);
    shader.SetUniformMat4f("model", model);
    
    /*shader.SetUniform3fv("material.color", material.color);
    shader.SetUniform1f("material.ao", material.ao);
    shader.SetUniform1f("material.metallic", material.metallic);
    shader.SetUniform1f("material.roughness", material.roughness);*/


    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
}

HitResult Sphere::intersect(Ray ray)
{
    HitResult res;

    glm::vec3 S = ray.start;
    glm::vec3 d = ray.dir;

    float OS = glm::distance(S, center);        //光源->圆心
    float SH = dot(center - S, d);              //沿dir方向
    float OH = sqrt(pow(OS, 2) - pow(SH, 2));   //OH 垂直于 SH

    // 不相交
    if (OH > R) 
        return res;


    float PH = sqrt(pow(R, 2) - pow(OH, 2));    //P为交点

    float t1 = SH - PH;
    float t2 = SH + PH;
    
    // 防止自交
    if (abs(t1) < 0.001f || abs(t2) < 0.001f)
        return res;

    float t = t1 < 0 ? t2 : t1;
    
    glm::vec3 P = S + t * d;

    res.isHit = true;
    res.distance = t;
    res.hitPoint = P;
    res.material = material;
    res.material.normal = glm::normalize(P - center);

    return res;
}

Ray::Ray(Light light)
{
    // TODO: set ray start and direction
    this->start = light.Pos;
    this->dir = glm::normalize(randomVec3());
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
        res[i].p1 = t.point0;
        res[i].p2 = t.point1;
        res[i].p3 = t.point2;

        // normal
        res[i].n1 = t.normal0;
        res[i].n2 = t.normal1;
        res[i].n3 = t.normal2;

        // material
        res[i].emissive = m.emissive;
        res[i].baseColor = m.color;
        res[i].param1 = glm::vec3(m.ao, m.roughness, m.metallic);
    }

    return res;
}