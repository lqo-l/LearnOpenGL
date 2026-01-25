#pragma once

#include <filesystem>
#include <vector>
#include <string>

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <mesh.hpp>
#include <utils.hpp>

namespace fs = std::filesystem;

class Model
{
public:
    Model(const std::string &path)
    {
        loadModel(path);
    }

    Model(const char *path)
    {
        loadModel(std::string(path));
    };

    void Draw(Shader &shader);
    const std::vector<Mesh> &getMeshes() const { return meshes; }
    std::vector<Mesh> &getMeshes() { return meshes; }

    void Debug(){
        std::cout<< "----------------------\nModel Dir: "<< directory << std::endl;
        std::cout << "Model has " << meshes.size() << " meshes." << std::endl;
        // for (size_t i = 0; i < meshes.size(); ++i) {
        //     std::cout << "Mesh " << i << " has " << meshes[i].vertices.size() << " vertices and " << meshes[i].indices.size() << " indices." << std::endl;
        // }
        std::cout<< "Textures loaded: " << textures_loaded.size() << std::endl;
        for(auto &tex : textures_loaded){
            std::cout << "Texture path: " << tex.path << ", id: " << tex.id << ", type: " << tex.type << std::endl;
        }
    }
private:
    std::vector<Mesh> meshes;
    std::vector<Texture> textures_loaded; // 记录加载过的纹理，避免每个mesh都加载重复的纹理
    std::string directory;                // 模型文件所在目录

    void loadModel(const std::string &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, const aiScene *scene); // 从模型文件中加载指定类型的纹理(路径或内嵌)，并创建gpu端纹理对象
};

inline void Model::Draw(Shader &shader)
{
    for (auto &mesh : meshes)
    {
        mesh.Draw(shader);
    }
}

inline void Model::loadModel(const std::string &path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    // const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate); // assimp加载模型时不设置翻转UV，在stb_image实际读取数据时翻转

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = fs::path(path).parent_path().string();

    processNode(scene->mRootNode, scene);
}

inline void Model::processNode(aiNode *node, const aiScene *scene)
{
    // 处理节点所有网格
    for (int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(processMesh(mesh, scene));
    }
    // 递归处理子节点
    for (int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

// 从aiMesh的Mesh数据结构中提取数据，生成自定义Mesh对象
inline Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    // 处理顶点
    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0])
        { // 有纹理坐标
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.TexCoords = {0.0f, 0.0f};
        }
        vertices.push_back(vertex);
    }
    // 处理索引
    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 处理材质
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient", scene);
        textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());

        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", scene);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

inline std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, const aiScene *scene)
{
    std::vector<Texture> textures;

    for (int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str; // 纹理路径
        mat->GetTexture(type, i, &str);
        // std::cout << "Texture path from assimp: " << str.C_Str() << std::endl;

        auto it = std::find_if(textures_loaded.begin(), textures_loaded.end(),
                               [&str](const Texture &tex)
                               {
                                   return tex.path == str.C_Str();
                               });
        if (it == textures_loaded.end())
        {
            Texture texture;
            // texture.id = TextureFromFile(str.C_Str(), this->directory);
            auto aitexture = scene->GetEmbeddedTexture(str.C_Str());
            if (aitexture)
            { // 对于内嵌纹理的类型，从AssImp中加载
                std::cout << "Loading embedded texture..." << std::endl;
                texture.id = loadTextureFromAssimp(aitexture);
            }
            else
            { // 非内嵌纹理，从文件系统加载
                std::string filePath = directory + '/' + std::string(str.C_Str());
                std::cout << "Loading external texture from: " << filePath << std::endl;
                texture.id = loadTexture(
                    filePath.c_str(),
                    GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,std::nullopt,
                    false // stb_image在这里不需要翻转，因为assimp已经翻转过UV坐标了
                );
            }

            if(texture.id == 0) {
                std::cout << "Failed to load texture: " << str.C_Str() << std::endl;
                continue;
            }

            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // 存储已加载的纹理
        }
        else
        {
            textures.push_back(*it);
        }
    }
    return textures;
}
