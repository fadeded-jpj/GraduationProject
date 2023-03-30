#include "BVH.h"
#include <algorithm>
#include <iostream>
#include <string>

const float _MAX_ = 114514;

using namespace glm;

int BuildBVH(std::vector<Triangle_encoded>& triangles, std::vector<BVHNode>& nodes, int left, int right, int n)
{
    if(left > right)
        return 0;
    
    nodes.push_back(BVHNode());
    int id = nodes.size() - 1;
    nodes[id].AA = glm::vec3(114514.0f);
    nodes[id].BB = glm::vec3(-114514.0f);
    nodes[id].index = nodes[id].left = nodes[id].right = nodes[id].n = 0;

    for (int i = left; i <= right; i++)
    {
        // AA
        float minX = min(triangles[i].p1.x, min(triangles[i].p2.x, triangles[i].p3.x));
        float minY = min(triangles[i].p1.y, min(triangles[i].p2.y, triangles[i].p3.y));
        float minZ = min(triangles[i].p1.z, min(triangles[i].p2.z, triangles[i].p3.z));
        nodes[id].AA.x = min(nodes[id].AA.x, minX);
        nodes[id].AA.y = min(nodes[id].AA.y, minY);
        nodes[id].AA.z = min(nodes[id].AA.z, minZ);

        // BB
        float maxX = max(triangles[i].p1.x, max(triangles[i].p2.x, triangles[i].p3.x));
        float maxY = max(triangles[i].p1.y, max(triangles[i].p2.y, triangles[i].p3.y));
        float maxZ = max(triangles[i].p1.z, max(triangles[i].p2.z, triangles[i].p3.z));
        nodes[id].BB.x = max(nodes[id].BB.x, maxX);
        nodes[id].BB.y = max(nodes[id].BB.y, maxY);
        nodes[id].BB.z = max(nodes[id].BB.z, maxZ);
    }

    //std::cout << "right = " << right << "   left = " << left << "  n = " << n << std::endl;

    if ((right - left + 1) <= n)
    {
        nodes[id].n = right - left + 1;
        nodes[id].index = left;
        return id;
    }

    // 划分
    // 先求最长轴
    float lenX = nodes[id].BB.x - nodes[id].AA.x;
    float lenY = nodes[id].BB.y - nodes[id].AA.y;
    float lenZ = nodes[id].BB.z - nodes[id].AA.z;

    // X
    if (lenX >= lenY && lenX >= lenZ)
        std::sort(triangles.begin() + left, triangles.begin() + right + 1,
            [](const Triangle_encoded& t1, const Triangle_encoded& t2) {
                return GetTriangleCenter(t1).x < GetTriangleCenter(t2).x;
            });
    // Y
    if (lenY >= lenX && lenY >= lenZ)
        std::sort(triangles.begin() + left, triangles.begin() + right + 1,
            [](const Triangle_encoded& t1, const Triangle_encoded& t2) {
                return GetTriangleCenter(t1).y < GetTriangleCenter(t2).y;
            });
    // Z
    if(lenZ >= lenX && lenZ >= lenY)
        std::sort(triangles.begin() + left, triangles.begin() + right + 1,
            [](const Triangle_encoded& t1, const Triangle_encoded& t2) {
                return GetTriangleCenter(t1).z < GetTriangleCenter(t2).z;
            });

    /*
    //SAH
    float Cost = _MAX_;
    int Axis = 0;
    int Split = (left + right) / 2;
    for (int axis = 0; axis < 3; axis++)
    {
        if(axis == 0)
            std::sort(&triangles[0] + left, &triangles[0] + right + 1,
                [](Triangle_encoded t1, Triangle_encoded t2) {
                    return GetTriangleCenter(t1).x < GetTriangleCenter(t2).x;
                });
        if(axis == 1)
            std::sort(&triangles[0] + left, &triangles[0] + right + 1,
                [](Triangle_encoded t1, Triangle_encoded t2) {
                    return GetTriangleCenter(t1).y < GetTriangleCenter(t2).y;
                });
        if(axis == 2)
            std::sort(&triangles[0] + left, &triangles[0] + right + 1,
                [](Triangle_encoded t1, Triangle_encoded t2) {
                    return GetTriangleCenter(t1).z < GetTriangleCenter(t2).z;
                });

        std::vector<glm::vec3> leftMax(right - left + 1, -1.0f * glm::vec3(_MAX_, _MAX_, _MAX_));
        std::vector<glm::vec3> leftMin(right - left + 1, glm::vec3(_MAX_, _MAX_, _MAX_));

   //    for (int i = left; i <= right; i++)
    //    {
    //        Triangle_encoded& t = triangles[i];
    //        int bias = ((i == left) ? 0 : 1);

   //        leftMax[i - left].x = std::max({ leftMax[i - left - bias].x, t.p1.x, t.p2.x, t.p3.x });
    //        leftMax[i - left].y = std::max({ leftMax[i - left - bias].y, t.p1.y, t.p2.y, t.p3.y });
    //        leftMax[i - left].z = std::max({ leftMax[i - left - bias].z, t.p1.z, t.p2.z, t.p3.z });
    //                     
    //        leftMin[i - left].x = std::min({ leftMin[i - left - bias].x, t.p1.x, t.p2.x, t.p3.x });
    //        leftMin[i - left].y = std::min({ leftMin[i - left - bias].y, t.p1.y, t.p2.y, t.p3.y });
    //        leftMin[i - left].z = std::min({ leftMin[i - left - bias].z, t.p1.z, t.p2.z, t.p3.z });
    //    }


        std::vector<glm::vec3> rightMax(right - left + 1, -1.0f * glm::vec3(_MAX_, _MAX_, _MAX_));
        std::vector<glm::vec3> rightMin(right - left + 1, glm::vec3(_MAX_, _MAX_, _MAX_));
        for (int i = right; i >= left; i--)
        {
            Triangle_encoded& t = triangles[i];
            int bias = (i == right) ? 0 : 1;  // 第一个元素特殊处理

            rightMax[i - left].x = std::max({ rightMax[i - left + bias].x, t.p1.x, t.p2.x, t.p3.x });
            rightMax[i - left].y = std::max({ rightMax[i - left + bias].y, t.p1.y, t.p2.y, t.p3.y });
            rightMax[i - left].z = std::max({ rightMax[i - left + bias].z, t.p1.z, t.p2.z, t.p3.z });

            rightMin[i - left].x = std::min({ rightMin[i - left + bias].x, t.p1.x, t.p2.x, t.p3.x });
            rightMin[i - left].y = std::min({ rightMin[i - left + bias].y, t.p1.y, t.p2.y, t.p3.y });
            rightMin[i - left].z = std::min({ rightMin[i - left + bias].z, t.p1.z, t.p2.z, t.p3.z });
        }

        float cost = _MAX_;
        int split = left;
        for (int i = left; i <= right-1; i++)
        {
            float lenx, leny, lenz;

            glm::vec3 leftAA = leftMin[i - left];
            glm::vec3 leftBB = leftMax[i - left];
            lenx = leftBB.x - leftAA.x;
            leny = leftBB.y - leftAA.y;
            lenz = leftBB.z - leftAA.z;
            float leftS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            float leftCost = leftS * (i - left + 1);

            // 右侧 [i+1, r]
            glm::vec3 rightAA = rightMin[i + 1 - left];
            glm::vec3 rightBB = rightMax[i + 1 - left];
            lenx = rightBB.x - rightAA.x;
            leny = rightBB.y - rightAA.y;
            lenz = rightBB.z - rightAA.z;
            float rightS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            float rightCost = rightS * (right - i);

            // 记录每个分割的最小答案
            float totalCost = leftCost + rightCost;
            if (totalCost < cost) {
                cost = totalCost;
                split = i;
            }
        }
        if (cost < Cost) {
            Cost = cost;
            Axis = axis;
            Split = split;
        }
    }

    if (Axis == 0) {
        std::sort(&triangles[0] + left, &triangles[0] + right + 1,
            [](Triangle_encoded t1, Triangle_encoded t2) {
                return GetTriangleCenter(t1).x < GetTriangleCenter(t2).x;
            });
    }
    else if (Axis == 1) {
        std::sort(&triangles[0] + left, &triangles[0] + right + 1,
            [](Triangle_encoded t1, Triangle_encoded t2) {
                return GetTriangleCenter(t1).y < GetTriangleCenter(t2).y;
            });
    }
    else {
        std::sort(&triangles[0] + left, &triangles[0] + right + 1,
            [](Triangle_encoded t1, Triangle_encoded t2) {
                return GetTriangleCenter(t1).z < GetTriangleCenter(t2).z;
            });
    }

    int l = BuildBVH(triangles, nodes, left, Split, n);
    int r = BuildBVH(triangles, nodes, Split + 1, right, n);
    */

    int mid = (left + right) / 2;
    int l = BuildBVH(triangles, nodes, left, mid, n);
    int r = BuildBVH(triangles, nodes, mid + 1, right, n);

    nodes[id].left = l;
    nodes[id].right = r;

    return id;
}

void myPrint(glm::vec3 v, std::string s)
{
    std::cout << s << ":(" << v.x << "," << v.y << "," << v.z << ")" << std::endl;
}

BVHNode_encode encodeBVH(const BVHNode& bvh)
{
    BVHNode_encode res;

    res.children = glm::vec3(bvh.left, bvh.right, 0);
    res.information = glm::vec3(bvh.n, bvh.index, 0);
    res.AA = bvh.AA;
    res.BB = bvh.BB;

    if (true) {
        myPrint(res.children, "childs");
        myPrint(res.information, "info");
        myPrint(res.AA, "AA");
        myPrint(res.BB, "BB");
        std::cout << std::endl;
    }
    return res;
}


