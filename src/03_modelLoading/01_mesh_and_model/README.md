# TODO
- [ ]  assmip加载pmx

# Problem
1. blender导出的fbx、glb格式几乎都没有成功导出纹理，原因不明。仅Princle BSDF的obj可以成功导出，但似乎纹理坐标错误无法正常显示
2. better fbx Exporter导出的obj和fbx都没有纹理
3. 中文的纹理路径名无法导入
4. mtl中有的纹理(hair.png)assimp没有加载进来:修改mtl的纹理路径名时，误将obj与mtl之间的索引名也替换掉了，二者需要对应。
5. 多个模型加载，texture id会累加：正常，glGenTextures返回的GLuint是GL上下文全局分配的，实际Draw时是根据实例中的索引绘制，不会出问题。
6. 纹理显示异常: Blender导出obj的可能就是异常的，重新导入Blender检查

# Conclusion
**唯一可用方式**：解包资源+复制+blender原生Obj导出+手动修改mtl数据和对应纹理文件名为英文

# Assimp
## Assimp数据结构
![Assimp数据结构](assimp.png "Assimp数据结构")

## 导入
```c++
Assimp::Importer importer;
const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs); 
```
导入选项：
`aiProcess_Triangulate`: 告诉Assimp，如果模型不是（全部）由三角形组成，它需要将模型所有的图元形状变换为三角形。
`aiProcess_FlipUVs`: 加载图像时，翻转y轴(OpenGL的t从底端向上增大，图像一般上方为t=0)
`aiProcess_GenNormals`：如果模型不包含法向量的话，就为每个顶点创建法线。
`aiProcess_SplitLargeMeshes`：将比较大的网格分割成更小的子网格，如果你的渲染有最大顶点数限制，只能渲染较小的网格，那么它会非常有用。
`aiProcess_OptimizeMeshes`：和上个选项相反，它会将多个小网格拼接为一个大的网格，减少绘制调用从而进行优化。

# 自定义数据结构
Model
    Mesh
        Vertex
        Texture



# 其他
aiProcess_FlipUVs: 翻转mTextureCoords中的v坐标为1-v
stbi_set_flip_vertically_on_load: 翻转实际的data数据data[y] = data[height - y]

# demo
![结果图](success.png)

# assimp纹理类型

```c++
enum aiTextureType {
    aiTextureType_NONE = 0,     
    aiTextureType_DIFFUSE = 1,  // map_Kd
    aiTextureType_SPECULAR = 2, // map_Ks
    aiTextureType_AMBIENT = 3,  // map_Ka
    aiTextureType_EMISSIVE = 4,
    aiTextureType_HEIGHT = 5,   // map_Bump
    aiTextureType_NORMALS = 6,  // map_Kn
    aiTextureType_SHININESS = 7,
    aiTextureType_OPACITY = 8,
    aiTextureType_DISPLACEMENT = 9, 
    aiTextureType_LIGHTMAP = 10,
    aiTextureType_REFLECTION = 11, // 并不会读取map_refl 
    ...
}

Assmip仅保留原始纹理路径或内嵌内存块，并不会因为纹理类型不同而进行其他解释。
所以可以任意读取使用
```