#pragma once

#include <glad/glad.h>

#include <string>
#include <vector>
#include <shader.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture{
    unsigned int id; // 纹理对象ID
    std::string type;  // texture_diffuse, texture_specular等自定义字符串
    std::string path;
};

class Mesh{
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures; // 存储相对于模型的路径

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    void Draw(Shader &shader);

private:
    unsigned int VAO, VBO, EBO;

    void setupMesh();
};


inline Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    : vertices(std::move(vertices)), indices(std::move(indices)), textures(std::move(textures)){
    setupMesh();
}

inline void Mesh::setupMesh(){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 属性
    glEnableVertexAttribArray(0); // 顶点
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1); // 法线
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal)); // offsetof自动计算成员相对于结构体起始位置的偏移，单位字节
    glEnableVertexAttribArray(2); // 纹理坐标
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0); // 解绑, 避免误操作
}

inline void Mesh::Draw(Shader &shader){
    // shader中用material结构体存储纹理
    // material中预设sampler2D类型的texture_diffuse1,texture_diffuse2,...,texture_specular1,...
    unsigned int diffuseIdx = 1;
    unsigned int specularIdx = 1;
    unsigned int normalIdx = 1;
    unsigned int ambientIdx = 1;
    for(unsigned int i = 0; i < textures.size(); i++){
        std::string name;
        std::string type = textures[i].type;
        std::string number;
        if(type == "texture_diffuse"){
            number = std::to_string(diffuseIdx++); 
        }else if(type == "texture_specular"){
            number = std::to_string(specularIdx++);
        }else if(type == "texture_normal"){
            number = std::to_string(normalIdx++);
        }else if(type == "texture_ambient"){
            number = std::to_string(ambientIdx++);
        }
        name = "material." + type + number;

        // 设置shader读取的纹理单元
        shader.setInt(name, i);
        // 设置纹理单元绑定的纹理对象
        glActiveTexture(GL_TEXTURE0 + i); 
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0); // 使用索引绘制
    glBindVertexArray(0); // 解绑

    glActiveTexture(GL_TEXTURE0); // 恢复默认纹理单元
}