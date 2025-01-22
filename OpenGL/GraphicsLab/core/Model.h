#pragma once

#include "Common.h"
#include "Mesh.h"
#include "ShaderProgram.h"

struct aiMesh;
struct aiNode;
struct aiScene;

class Model 
{
public:
    static std::unique_ptr<Model> Load(std::filesystem::path filepath);

    size_t GetMeshCount() const { return m_meshes.size(); }
    std::shared_ptr<StaticMesh> GetMesh(int index) const { return m_meshes[index]; }
    void Draw(const gl::ShaderProgram* program) const;

private:
    Model() {}
    
    bool LoadByAssimp(std::filesystem::path filepath);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene);
    void ProcessNode(aiNode* node, const aiScene* scene);
        
    std::vector<std::shared_ptr<StaticMesh>> m_meshes;
    std::vector<std::shared_ptr<Material>> m_materials;
};