#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texCoord;
};

struct Mesh
{
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	uint32_t m_vbo{ 0 };
	uint32_t m_ebo{ 0 };

	~Mesh();
	void UpdateBuffer();
	void Draw();
};

Mesh CreateCubeMesh();
Mesh CreateCylinderMesh(float radius, float height);
Mesh CreateSphereMesh(float radius);
Mesh CreateHemisphereMesh(float radius);
Mesh CreateTorusMesh(float radius, float tubeRadius);

Mesh CreatePlaneMesh();