#pragma once

#include "Common.h"
#include "Buffer.h"
#include "VertexLayout.h"
#include "Texture.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

struct Mesh
{
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
};

class Material
{
public:
	static std::unique_ptr<Material> Create();

	std::shared_ptr<gl::Texture> m_diffuseTex;
	std::shared_ptr<gl::Texture> m_specularTex;
};

class StaticMesh
{
public:
	StaticMesh() = default;
	StaticMesh(const Mesh& mesh);
	void UpdateBuffer();
	void Draw();

	Mesh m_mesh;
	gl::VertexLayout m_vertexLayout;
	std::unique_ptr<gl::Buffer> m_vertexBuffer;
	std::unique_ptr<gl::Buffer> m_indexBuffer;
	std::shared_ptr<Material> m_material;
};

Mesh CreateCubeMesh();
Mesh CreateCylinderMesh(float radius, float height);
Mesh CreateSphereMesh(float radius);
Mesh CreateHemisphereMesh(float radius);
Mesh CreateTorusMesh(float radius, float tubeRadius);
Mesh CreatePlaneMesh();