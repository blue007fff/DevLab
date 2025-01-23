#include "Model.h"
#include "Texture.h"
#include "Image.h"
#include "ShaderProgram.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


std::unique_ptr<Model> Model::Load(std::filesystem::path filepath)
{
	auto model = std::unique_ptr<Model>(new Model());
	if (!model->LoadByAssimp(filepath))
		return nullptr;
	return std::move(model);
}

bool Model::LoadByAssimp(std::filesystem::path filepath)
{
	Assimp::Importer importer;
	auto scene = importer.ReadFile(filepath.string(),
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
		aiProcess_GenUVCoords | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		SPDLOG_ERROR("failed to load model: {}", filepath.string());
		return false;
	}	

	std::filesystem::path folderPath = filepath.parent_path();
	auto LoadTexture = [&](aiMaterial* material, aiTextureType type) -> std::unique_ptr<gl::Texture>
	{
		if (material->GetTextureCount(type) <= 0)
			return nullptr;

		aiString texFilename;
		material->GetTexture(type, 0, &texFilename);
		std::filesystem::path texFilepath = folderPath / texFilename.C_Str();
		auto image = Image::Create(texFilepath.string(), true);
		auto tex = gl::Texture::Create(image.get());
		return tex;
	};

	for (uint32_t i = 0; i < scene->mNumMaterials; i++)
	{
		auto material = scene->mMaterials[i];
		auto glMaterial = Material::Create();
		glMaterial->m_diffuseTex = LoadTexture(material, aiTextureType_DIFFUSE);
		glMaterial->m_specularTex = LoadTexture(material, aiTextureType_SPECULAR);
		m_materials.push_back(std::move(glMaterial));
	}

	ProcessNode(scene->mRootNode, scene);
	return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; i++) {
		auto meshIndex = node->mMeshes[i];
		auto mesh = scene->mMeshes[meshIndex];
		ProcessMesh(mesh, scene);
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++) {
		ProcessNode(node->mChildren[i], scene);
	}
}

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	SPDLOG_INFO("process mesh: {}, #vert: {}, #face: {}",
		mesh->mName.C_Str(), mesh->mNumVertices, mesh->mNumFaces);

	std::vector<Vertex> vertices;
	vertices.resize(mesh->mNumVertices);
	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		auto& v = vertices[i];
		v.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		if(mesh->mTextureCoords[0])
			v.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
	}

	std::vector<uint32_t> indices;
	indices.resize(mesh->mNumFaces * 3);
	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		indices[3 * i] = mesh->mFaces[i].mIndices[0];
		indices[3 * i + 1] = mesh->mFaces[i].mIndices[1];
		indices[3 * i + 2] = mesh->mFaces[i].mIndices[2];
	}

	std::shared_ptr<StaticMesh> myMesh(new StaticMesh());
	myMesh->m_mesh.m_vertices = vertices;
	myMesh->m_mesh.m_indices = indices;
	myMesh->UpdateBuffer();	
	if (mesh->mMaterialIndex >= 0)
		myMesh->m_material = m_materials[mesh->mMaterialIndex];

	m_meshes.push_back(std::move(myMesh));
}


void Model::Draw(const gl::ShaderProgram* program) const
{
	for (auto& mesh : m_meshes) {
		mesh->Draw();
	}
}