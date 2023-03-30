#pragma once
#include "Mesh.h"
#include "Shader.h"

#include <vector>
#include <string>
#include <unordered_set>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


class Model
{
private:
	std::vector<Mesh> m_Meshes;
	std::string m_Directory;

	std::unordered_set<std::string> m_Textures_loaded;

public:
	Model(const std::string& filePath);
	~Model() {}

	void Draw(Shader shader);

private:
	void loadModel(const std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

unsigned int TextureFromFile(const std::string& path, const std::string& directory, bool gamma = false);