#include "Mesh.h"
#include <glad/gl.h>

#pragma warning(push)            // 현재 경고 상태 저장
#pragma warning(disable : 4996)  // C4996: 사용되지 않음(deprecated) 경고 비활성화
#pragma warning(disable : 4305)  // C4305: 데이터 손실 경고 비활성화 (축소 변환)
#pragma warning(disable : 4244)  // C4244: 데이터 손실 경고 비활성화 (정수/실수 변환)
#define PAR_SHAPES_IMPLEMENTATION
#include "par_shapes.h"
#pragma warning(pop)

static Mesh CreateMeshFromParShape(par_shapes_mesh* shape);

StaticMesh::StaticMesh(const Mesh& mesh)
	: m_mesh{ mesh }
{
}

void StaticMesh::UpdateBuffer()
{
	m_vertexLayout.SetAttrib(0, GL_FLOAT, 3, sizeof(Vertex), 0);
	m_vertexLayout.SetAttrib(1, GL_FLOAT, 3, sizeof(Vertex), offsetof(Vertex, normal));
	m_vertexLayout.SetAttrib(2, GL_FLOAT, 2, sizeof(Vertex), offsetof(Vertex, texCoord));

	if (m_vertexBuffer == nullptr)
	{
		m_vertexBuffer = gl::Buffer::CreateWithData(
			gl::Buffer::Type::ARRAY, gl::Buffer::Usage::STATIC,
			m_mesh.m_vertices.data(), sizeof(Vertex), m_mesh.m_vertices.size());
	}
	if (m_indexBuffer == nullptr)
	{
		m_indexBuffer = gl::Buffer::CreateWithData(
			gl::Buffer::Type::ELEMENT_ARRAY, gl::Buffer::Usage::STATIC,
			m_mesh.m_indices.data(), sizeof(uint32_t), m_mesh.m_indices.size());
	}
}

void StaticMesh::Draw()
{
	m_vertexBuffer->Bind();
	m_indexBuffer->Bind();
	m_vertexLayout.Enable();

	glDrawElements(GL_TRIANGLES, (uint32_t)m_mesh.m_indices.size(), GL_UNSIGNED_INT, (void*)0);
}

Mesh CreateCubeMesh()
{
	int xSlices{ 4 }, yslices{ 4 };
	float x[]{ 1, 0, 0 }, y[]{ 0, 1, 0 }, z[]{ 0, 0, 1 };
	par_shapes_mesh* shapePz = par_shapes_create_plane(xSlices, yslices);
	par_shapes_translate(shapePz, -0.5f, -0.5f, 0.5f);

	par_shapes_mesh* shapeMz = par_shapes_clone(shapePz, nullptr);
	par_shapes_rotate(shapeMz, float(PAR_PI), y);

	par_shapes_mesh* shapeMy = par_shapes_clone(shapePz, nullptr);
	par_shapes_rotate(shapeMy, float(PAR_PI * 0.5), x);

	par_shapes_mesh* shapePx = par_shapes_clone(shapeMy, nullptr);
	par_shapes_rotate(shapePx, float(PAR_PI * 0.5), z);

	par_shapes_mesh* shapePy = par_shapes_clone(shapePx, nullptr);
	par_shapes_rotate(shapePy, float(PAR_PI * 0.5), z);

	par_shapes_mesh* shapeMx = par_shapes_clone(shapePy, nullptr);
	par_shapes_rotate(shapeMx, float(PAR_PI * 0.5), z);

	par_shapes_mesh* shape = shapePz;
	par_shapes_merge_and_free(shape, shapeMz);
	par_shapes_merge_and_free(shape, shapeMy);
	par_shapes_merge_and_free(shape, shapePx);
	par_shapes_merge_and_free(shape, shapePy);
	par_shapes_merge_and_free(shape, shapeMx);

	Mesh mesh = CreateMeshFromParShape(shape);
	par_shapes_free_mesh(shape);
	return mesh;


#if 0
	Mesh mesh;
	float size = 0.5f;
	mesh.m_vertices = std::vector<Vertex>{
		// Positive X side
	   {{ size,  size,  size}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
	   {{ size,  size, -size}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	   {{ size, -size,  size}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
	   {{ size, -size, -size}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

	   // Negative X side
	   {{-size,  size,  size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
	   {{-size,  size, -size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	   {{-size, -size,  size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
	   {{-size, -size, -size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

	   // Positive Y side
	   {{ size,  size,  size}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
	   {{ size,  size, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	   {{-size,  size,  size}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
	   {{-size,  size, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},

	   // Negative Y side
	   {{ size, -size,  size}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
	   {{ size, -size, -size}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	   {{-size, -size,  size}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
	   {{-size, -size, -size}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},

	   // Positive Z side
	   {{ size,  size,  size}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	   {{-size,  size,  size}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
	   {{ size, -size,  size}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	   {{-size, -size,  size}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

	   // Negative Z side
	   {{ size,  size, -size}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	   {{-size,  size, -size}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
	   {{ size, -size, -size}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	   {{-size, -size, -size}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
	};

	mesh.m_indices = std::vector<uint32_t>{
		 0, 2, 1, 2, 3, 1,          // Positive X side         
		 4, 5, 6, 5, 7, 6,          // Negative X side         
		 8, 9, 10, 10, 9, 11,       // Positive Y side         
		 12, 14, 13, 13, 14, 15,    // Negative Y side         
		 16, 17, 18, 18, 17, 19,    // Positive Z side         
		 20, 22, 21, 21, 22, 23  // Negative Z side
	};

	return mesh;
#endif


}

Mesh CreateCylinderMesh(float radius, float height)
{
	int slices{ 36 }, heightStacks{ 4 };
	par_shapes_mesh* shape = par_shapes_create_cylinder(slices, heightStacks);
	for (int i = 0; i < shape->npoints; ++i)
	{
		std::swap(shape->tcoords[i * 2 + 0], shape->tcoords[i * 2 + 1]);
		shape->tcoords[i * 2 + 0] = 1.0f - shape->tcoords[i * 2 + 0];
	}
	par_shapes_scale(shape, radius, radius, height);
	

	par_shapes_mesh* topCircleShape = par_shapes_create_hemisphere(3, slices);
	float x[]{ 1, 0, 0 }, y[]{0, 1, 0}, z[]{ 0, 0, 1 };
	par_shapes_rotate(topCircleShape, float(PAR_PI / 2.0), x);
	par_shapes_rotate(topCircleShape, -float(PAR_PI / 2.0), z);
	par_shapes_scale(topCircleShape, radius, radius, 0.0);

	par_shapes_mesh* bottomPlaneShape = par_shapes_clone(topCircleShape, nullptr);
	par_shapes_rotate(bottomPlaneShape, float(PAR_PI), y);
	par_shapes_merge_and_free(shape, bottomPlaneShape);

	par_shapes_translate(topCircleShape, 0.0, 0.0, height);
	par_shapes_merge_and_free(shape, topCircleShape);

	Mesh mesh = CreateMeshFromParShape(shape);
	par_shapes_free_mesh(shape);
	return mesh;
}

Mesh CreateSphereMesh(float radius)
{
	int slices{ 36 }, zStatcks{ 18 };
	par_shapes_mesh* shape = par_shapes_create_parametric_sphere(slices, zStatcks);
	par_shapes_scale(shape, radius, radius, radius);
	//float axis[]{ 0, 1, 0 };
	//par_shapes_rotate(shape, float(PAR_PI / 5.0), axis);

	Mesh mesh = CreateMeshFromParShape(shape);
	for (auto& v : mesh.m_vertices)
	{
		std::swap(v.texCoord.x, v.texCoord.y);
		v.texCoord.y = 1.0f - v.texCoord.y;
	}
	par_shapes_free_mesh(shape);
	return mesh;
}

Mesh CreatePlaneMesh()
{
	par_shapes_mesh* shape = par_shapes_create_plane(1, 1);
	par_shapes_translate(shape, -0.5f, -0.5f, 0.0f);
	Mesh mesh = CreateMeshFromParShape(shape);
	par_shapes_free_mesh(shape);
	return mesh;

#if 0
	Mesh mesh;
	float size = 1.0f;
	float width = size;
	float height = size;

	mesh.m_vertices = std::vector<Vertex>{
		{{-width / 2, -height / 2, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 좌하
		{{ width / 2, -height / 2, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 우하
		{{ width / 2,  height / 2, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 우상
		{{-width / 2,  height / 2, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 좌상
	};

	mesh.m_indices = std::vector<uint32_t>{
		0, 1, 2, 0, 2, 3,
	};

	return mesh;
#endif

}

Mesh CreateMeshFromParShape(par_shapes_mesh* shape)
{
	Mesh mesh;
	mesh.m_vertices.resize(shape->npoints);
	mesh.m_indices.resize(shape->ntriangles * 3);

	for (int i = 0; i < shape->npoints; ++i)
	{
		Vertex v;
		v.pos = glm::vec3(shape->points[i * 3 + 0], shape->points[i * 3 + 1], shape->points[i * 3 + 2]);
		v.normal = glm::vec3(shape->normals[i * 3 + 0], shape->normals[i * 3 + 1], shape->normals[i * 3 + 2]);
		v.texCoord = glm::vec2(shape->tcoords[i * 2 + 0], shape->tcoords[i * 2 + 1]);
		mesh.m_vertices[i] = v;
	}
	for (int i = 0; i < shape->ntriangles; ++i)
	{
		mesh.m_indices[i * 3 + 0] = shape->triangles[i * 3 + 0];
		mesh.m_indices[i * 3 + 1] = shape->triangles[i * 3 + 1];
		mesh.m_indices[i * 3 + 2] = shape->triangles[i * 3 + 2];
	}
	return mesh;
	}

Mesh CreateHemisphereMesh(float radius)
{
	int slices{ 36 }, zStacks{ 9 };
	par_shapes_mesh* shape = par_shapes_create_hemisphere(slices, zStacks);
	
	par_shapes_scale(shape, radius, radius, radius);
	float x[]{ 1, 0, 0 }, z[]{ 0, 0, 1 };
	par_shapes_rotate(shape, float(PAR_PI / 2.0), x);
	par_shapes_rotate(shape, -float(PAR_PI / 2.0), z);

	{
		par_shapes_mesh* bottomPlaneShape = par_shapes_create_hemisphere(3, slices);
		float x[]{ 1, 0, 0 }, y[]{ 0, 1, 0 }, z[]{ 0, 0, 1 };
		par_shapes_rotate(bottomPlaneShape, float(PAR_PI / 2.0), x);
		par_shapes_rotate(bottomPlaneShape, -float(PAR_PI / 2.0), z);
		par_shapes_scale(bottomPlaneShape, radius, radius, 0.0);
		par_shapes_rotate(bottomPlaneShape, float(PAR_PI), y);
		par_shapes_merge_and_free(shape, bottomPlaneShape);
	}

	Mesh mesh = CreateMeshFromParShape(shape);
	par_shapes_free_mesh(shape);
	return mesh;
}

#if 0
static void par_shapes__circleplane(float const* uv, float* xyz, void* userdata)
{
	/*float r = uv[1] * 0.5f + 0.5f;
	float theta = uv[0] * 2 * PAR_PI;
	xyz[0] = r * sinf(theta);
	xyz[1] = r * cosf(theta);
	xyz[2] = 0.0f;*/
	float r = uv[1] * 0.5f + 0.5f;
	float theta = uv[0] * 2 * PAR_PI;
	xyz[0] = r * sinf(theta);
	xyz[1] = r * cosf(theta);
	xyz[2] = 0.0f;
}

par_shapes_mesh* par_shapes_create_mydisk(int slices, int stacks)
{
	if (slices < 3 || stacks < 3) {
		return 0;
	}
	par_shapes_mesh* m = par_shapes_create_parametric(par_shapes__circleplane,
		slices, stacks, 0);
	par_shapes_remove_degenerate(m, par_shapes__epsilon_degenerate_sphere);
	return m;
}
#endif // 0


Mesh CreateTorusMesh(float radius, float tubeRadius)
{
	assert(radius > tubeRadius);
	int slices{ 36 }, zStacks{ 36 };
	par_shapes_mesh* shape = par_shapes_create_torus(slices, zStacks, tubeRadius / radius);
	par_shapes_scale(shape, radius, radius, radius);
	Mesh mesh = CreateMeshFromParShape(shape);

	//for (auto& v : mesh.m_vertices)
	//{
	//    //std::swap(v.texCoord.x, v.texCoord.y);// v.pos.z = 0.0f;
	//    //v.texCoord.y = 1.0f - v.texCoord.y;
	//}
	par_shapes_free_mesh(shape);
	return mesh;
}