#include "Model.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include "stb_image/stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Shape.h"
#include "encoded/Triangle.h"

using namespace glm;

Model::Model(const std::string& filePath)
{
    loadModel(filePath);
}

void Model::Draw(Shader shader)
{
    for (auto& mesh : m_Meshes)
        mesh.Draw(shader);
}

void Model::loadModel(const std::string path)
{
    Assimp::Importer importer;
    //ReadFile(path, 后期处理(不是三角形的变成三角形|y轴翻转))
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    m_Directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // 处理节点所有的网格（如果有的话）
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_Meshes.push_back(processMesh(mesh, scene));
    }
    // 接下来对它的子节点重复这一过程
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<MeshTexture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // TODO: caculate the vertex pos、normal and texcoord
        vertex.Position = glm::vec3(mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z);

        vertex.Normal = glm::vec3(mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0])
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // TODO: caculate indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // TODO: caculate material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material,
            aiTextureType_DIFFUSE, "texture_diffuse");

        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<MeshTexture> specularMaps = loadMaterialTextures(material,
            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<MeshTexture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<MeshTexture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;

        // aiString 没有实现哈希散列函数 不能直接使用
        if (m_Textures_loaded.find(str.C_Str()) != m_Textures_loaded.end())
            continue;

        MeshTexture texture;
        texture.id = TextureFromFile(str.C_Str(), m_Directory);
        texture.type = typeName;
        texture.path = str;
        textures.push_back(texture);
        m_Textures_loaded.emplace(str.C_Str());
    }
    return textures;
}

unsigned int TextureFromFile(const std::string& path, const std::string& directory, bool gamma)
{
    std::string filename = path;
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//// 读取 obj
//void readObj(std::string filepath, std::vector<Triangle>& triangles, Material material, mat4 trans, bool smoothNormal) {
//
//    // 顶点位置，索引
//    std::vector<vec3> vertices;
//    std::vector<GLuint> indices;
//
//    // 打开文件流
//    std::ifstream fin(filepath);
//    std::string line;
//    if (!fin.is_open()) {
//        std::cout << "文件 " << filepath << " 打开失败" << std::endl;
//        exit(-1);
//    }
//
//    // 计算 AABB 盒，归一化模型大小
//    float maxx = -11451419.19;
//    float maxy = -11451419.19;
//    float maxz = -11451419.19;
//    float minx = 11451419.19;
//    float miny = 11451419.19;
//    float minz = 11451419.19;
//
//    // 按行读取
//    while (std::getline(fin, line)) {
//        std::istringstream sin(line);   // 以一行的数据作为 string stream 解析并且读取
//        std::string type;
//        GLfloat x, y, z;
//        int v0, v1, v2;
//        int vn0, vn1, vn2;
//        int vt0, vt1, vt2;
//        char slash;
//
//        // 统计斜杆数目，用不同格式读取
//        int slashCnt = 0;
//        for (int i = 0; i < line.length(); i++) {
//            if (line[i] == '/') slashCnt++;
//        }
//
//        // 读取obj文件
//        sin >> type;
//        if (type == "v") {
//            sin >> x >> y >> z;
//            vertices.push_back(vec3(x, y, z));
//            maxx = max(maxx, x); maxy = max(maxx, y); maxz = max(maxx, z);
//            minx = min(minx, x); miny = min(minx, y); minz = min(minx, z);
//        }
//        if (type == "f") {
//            if (slashCnt == 6) {
//                sin >> v0 >> slash >> vt0 >> slash >> vn0;
//                sin >> v1 >> slash >> vt1 >> slash >> vn1;
//                sin >> v2 >> slash >> vt2 >> slash >> vn2;
//            }
//            else if (slashCnt == 3) {
//                sin >> v0 >> slash >> vt0;
//                sin >> v1 >> slash >> vt1;
//                sin >> v2 >> slash >> vt2;
//            }
//            else {
//                sin >> v0 >> v1 >> v2;
//            }
//            indices.push_back(v0 - 1);
//            indices.push_back(v1 - 1);
//            indices.push_back(v2 - 1);
//        }
//    }
//
//    // 模型大小归一化
//    float lenx = maxx - minx;
//    float leny = maxy - miny;
//    float lenz = maxz - minz;
//    float maxaxis = max(lenx, max(leny, lenz));
//    for (auto& v : vertices) {
//        v.x /= maxaxis;
//        v.y /= maxaxis;
//        v.z /= maxaxis;
//    }
//
//    // 通过矩阵进行坐标变换
//    for (auto& v : vertices) {
//        vec4 vv = vec4(v.x, v.y, v.z, 1);
//        vv = trans * vv;
//        v = vec3(vv.x, vv.y, vv.z);
//    }
//
//    // 生成法线
//    std::vector<vec3> normals(vertices.size(), vec3(0, 0, 0));
//    for (int i = 0; i < indices.size(); i += 3) {
//        vec3 p1 = vertices[indices[i]];
//        vec3 p2 = vertices[indices[i + 1]];
//        vec3 p3 = vertices[indices[i + 2]];
//        vec3 n = normalize(cross(p2 - p1, p3 - p1));
//        normals[indices[i]] += n;
//        normals[indices[i + 1]] += n;
//        normals[indices[i + 2]] += n;
//    }
//
//    // 构建 Triangle 对象数组
//    int offset = triangles.size();  // 增量更新
//    triangles.resize(offset + indices.size() / 3);
//    for (int i = 0; i < indices.size(); i += 3) {
//        Triangle& t = triangles[offset + i / 3];
//        // 传顶点属性
//        t.p1 = vertices[indices[i]];
//        t.p2 = vertices[indices[i + 1]];
//        t.p3 = vertices[indices[i + 2]];
//        if (!smoothNormal) {
//            vec3 n = normalize(cross(t.p2 - t.p1, t.p3 - t.p1));
//            t.n1 = n; t.n2 = n; t.n3 = n;
//        }
//        else {
//            t.n1 = normalize(normals[indices[i]]);
//            t.n2 = normalize(normals[indices[i + 1]]);
//            t.n3 = normalize(normals[indices[i + 2]]);
//        }
//
//        // 传材质
//        t.material = material;
//    }
//}
