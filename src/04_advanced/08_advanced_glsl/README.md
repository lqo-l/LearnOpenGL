## glsl built-in 变量
> [wiki: built-in variable](https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL))

### 顶点着色器:
gl_Position: 顶点着色器的裁剪空间坐标,尚未进行透视除法
gl_PointSize: 图元为GL_POINTS时点的大小, 启用`glEnable(GL_PROGRAM_POINT_SIZE)`后才可在着色器中修改.
gl_VertexID: 顶点ID。对于 indexed draw，是索引数组的值；
对于 non-indexed draw，是从 first 开始递增的值。

### 片段着色器:
gl_FragCoord: 片段着色器的opengl窗口坐标,深度[0,1],左下角(0,0),[0,w],[0,h]
gl_FrontFacing: 一个面是正向还是背向面,正向为true
gl_PointCoord: 点图元内的归一化坐标, (0,0) 到 (1,1)
gl_FragDepth: 可用于修改片段深度(如果着色器没有写入值到gl_FragDepth，它会自动取用gl_FragCoord.z的值),写入会导致关闭early-z. OpenGL 4.2以上可以调和限定修改的范围.
    `layout (depth_<condition>) out float gl_FragDepth;`

    | condition | description |
    | --- | --- |
    | any | 默认值。提前深度测试是禁用的，你会损失很多性能 |
    | greater | 你只能让深度值**比gl_FragCoord.z**更大 |
    | less | 你只能让深度值比gl_FragCoord.z更小 |
    | unchanged | 如果你要写入gl_FragDepth，你将只能写入gl_FragCoord.z的值 |


## 接口块
类似结构体,out打包了发送到下一个着色器的所有变量

```glsl
/// .vert
out VS_OUT // 块名
{
    vec2 TexCoords;
} vs_out;

void main(){
    vs_out.TexCoords = aTexCoords; // 访问
}

/// .frag
in VS_OUT //  块名应保持一致
{
    vec2 TexCoords;
} fs_in;    // 实例名任意

void main(){
    FragColor = texture(tex, fs_in.TexCoords); // 访问
}
```

## uniform缓冲对象(ubo)
**特点**：常量内存区；shader端不可写；单buffer64KB限制
**作用场景**：当不同shader程序使用相同的uniform变量时,可以使用uniform buffer,只需设置一次,如view和projection.
### 使用
创建:glGenBuffers
绑定:GL_UNIFORM_BUFFER缓冲目标
分配内存：
```c++
// 可以直接分配两个mat4的内存空间
glBindBuffer(GL_UNIFORM_BUFFER,UBO);
glBufferData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4), nullptr, GL_STATIC_DRAW); 

// 或者根据实际uniform块大小分配空间（编译器把对齐、填充都算完后的真实字节数，较准确）
unsigned int index = glGetUniformBlockIndex(shaderA.ID, "UniformBlockName");   // 获取uniform块在shader中的索引
GLint bufferSize = 0;
glGetActiveUniformBlockiv(programObject[0], index, GL_UNIFORM_BLOCK_DATA_SIZE, &bufferSize);//获取Uniform块的大小
glBindBuffer(GL_UNIFORM_BUFFER,UBO);
glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_STATIC_DRAW); // 根据获取到的bufferSize分配内存

```

例:
```glsl
layout(std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};
void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0); // 直接访问即可,不用加块名
}
```
> shader中的叫uniform block， gpu存储中的叫uniform buffer。 block从buffer拿数据


### 块布局
uniform中的变量在内存中按一定规则存储,称为块布局。std140是其中一种最容易使用的块布局.

**std140规则:**
1. **基准对齐量（base alignment）**:变量占据的空间(含Padding). 
- 标量： base alignment = 4 bytes
- vec2： base alignment = 8 bytes
- vec3/vec4: base alignment = 16 bytes
- 数组： 每个元素的 base alignment 向上取整到 16 bytes
- 矩阵： 等价于 列向量数组 每列 base alignment = 16 bytes
- 结构体整体对齐到16B


1. **对齐偏移量（offset）**:变量相对于块起始位置的字节偏移量. 
- **必须是基准对齐量的倍数**
- 任意非标量（向量、数组）的对齐偏移量为16，如vec2
> 所有向量都被假定为从一个 vec4 槽位中读取, 是 std140 为硬件访问模式做的设计取舍

  
例：
``` glsl
layout (std140) uniform ExampleBlock
{
                     // 基准对齐量       // 对齐偏移量
    float value;     // 4               // 0 
    vec3 vector;     // 16              // 16  (必须是16的倍数，所以 4->16)
    mat4 matrix;     // 16              // 32  (列 0)
                     // 16              // 48  (列 1)
                     // 16              // 64  (列 2)
                     // 16              // 80  (列 3)
    float values[3]; // 16              // 96  (values[0])
                     // 16              // 112 (values[1])
                     // 16              // 128 (values[2])
    bool boolean;    // 4               // 144
    int integer;     // 4               // 148
}; 
```

**其他布局**
**shared(默认布局):** shared 布局允许驱动自由安排内存布局，需要通过 API 查询 offset。 只能使用`glGetUniformIndices`+`glGetActiveUniformsiv `获取指定uniform变量的偏移量

```c++
// 查询索引，原型
void glGetUniformIndices(
    GLuint program,
    GLsizei uniformCount,
    const GLchar *const *uniformNames,
    GLuint *uniformIndices
); // 获取uniform 变量索引（注意不是uniform block的索引）

// 使用例
const GLchar* names[] = { "viewMatrix", "projMatrix" };
GLuint indices[2];
glGetUniformIndices(program, 2, names, indices);
// indices[0] = viewMatrix 的 uniform index

// 根据索引查询属性，原型
void glGetActiveUniformsiv(
    GLuint program,
    GLsizei uniformCount,
    const GLuint *uniformIndices,
    GLenum pname, // GL_UNIFORM_TYPE，GL_UNIFORM_SIZE，GL_UNIFORM_BLOCK_INDEX，GL_UNIFORM_OFFSET（UBO成员才有偏移,否则返回-1或无意义值）
    GLint *params // 属性值
);

// 使用例
GLint offset;
if (uniformIndices[0] != GL_INVALID_INDEX){ // 检查索引是否有效
    glGetActiveUniformsiv(program, 1, &uniformIndices[0], GL_UNIFORM_OFFSET, &offset); // 得到offset
}
```



>**何时使用**: 如果需要在c端写一个struct,通过`glBufferData`/`glBufferSubData`一次性拷贝整块内存到Uniform Block,需要内存对齐,必须选std140 ,否则uniform block的内存被调整后,可能与结构体定义的内存不一致.
> 如果只是一个一个变量的设置,两种效果一致,区别是shared不能预先估计偏移量,要通过api获取


### 绑定点
通过绑定点(Bind Points)让不同shader程序的uniform块使用同一块uniform buffer数据
1. ubo对象绑定到绑定点
```c++
glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboExampleBlock);  // 绑定点2
// 或
glBindBufferRange(GL_UNIFORM_BUFFER, 2, uboExampleBlock, 0, 152); // 绑定点2使用部分uniform buffer数据
```
注：range绑定时，offset(倒数第二个形参)必须是`GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT`的倍数，为了硬件高效读取。可以通过以下api查询。我这里输出为256
```C++
GLint uniformBufferOffsetAlign = 0;
glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBufferOffsetAlign);
std::cout<< "uniform buffer offset alignment: " << uniformBufferOffsetAlign << std::endl; // glBindBufferRange的offset必须是该值的整数倍
```

2. shader中ubo块绑定到绑定点
```c++
unsigned int index = glGetUniformBlockIndex(shaderA.ID, "UniformBlockName");   
glUniformBlockBinding(shaderA.ID, index, 2); //  绑定点2
```
或OpenGL4.2起
```glsl
layout(std140, binding = 2) uniform UniformBlockName { ... }; // 绑定点2
```
同时设置，以cpu端的设置为准（最后一次设置）
> 1. 编译 shader
> 2. link program → layout(binding = 2) 生效
> 3. glUseProgram(program)
> 4. glUniformBlockBinding(program, blockIndex, 3) → 绑定点被改为 3（覆盖 layout）


### uniform buffer通过偏移量填充uniform block数据
以上文的std140块布局的uboExampleBlock为例,通过偏移量填充其**144对齐偏移量位置**的bool值
```c++
glBindBuffer(GL_UNIFORM_BUFFER, uboExampleBlock);
int b = true; // GLSL中的bool是4字节的，所以我们将它存为一个integer
glBufferSubData(GL_UNIFORM_BUFFER, 144, 4, &b); 
glBindBuffer(GL_UNIFORM_BUFFER, 0);
```
上面的方法可以一次性填充多个变量(一个结构体).

过去用于更新uniform scalar的方式不可用于更新ubuffer。UBO 中的 uniform 永远没有 location，glGetUniformLocation 永远返回 -1；
```c++
GLint loc = glGetUniformLocation(prog, "boolean");
glUniform1i(loc, GL_TRUE);
```

## Shader Storage Buffer Object(SSBO)
**特点**：全局内存区VRAM；shader端可读可写；不限存储；跨Pass共享；原子操作（Computer Shader）
> 对比UBO：UBO有常量缓存加速，但容量小，不可写。
> 不确定：跨Pass需要重新拷贝block到常量缓存。

### std430
SSBO使用std430布局，更省内存。 成员按声明顺序布局，每个成员按自身对齐要求放置

1. 基准对齐量
- 标量：（float, int, uint, bool） base alignment = 4 bytes
- vec2: base alignment = 8 bytes
- vec3/vec4: base alignment = 16 bytes
> 注意：vec3 仍为 16 字节对齐（这是规范强制要求，不是 12）
- **数组：每个元素的 base alignment 保持其原始对齐量（不再强制 16 字节）**
- 结构体整体 base alignment = 其所有成员中最大的 base alignment 

2. 对齐偏移量
- 必须是其基准对齐量的倍数
- **数组元素的偏移量为其元素的基准对齐量**
- vec2的对齐偏移量是8B而非16B（vec3和vec4一样是16B）

>核心思想：去掉 std140 中“所有非标量强制 16B 对齐”的限制，允许自然对齐，但保留 vec3/vec4 的 16B 对齐（出于硬件效率）

例：
```glsl
layout (std430) buffer ExampleBlock
{
                     // 基准对齐量       // 对齐偏移量
    float value;     // 4               // 0 
    vec3 vector;     // 16              // 16  (4 → 对齐到16)
    mat4 matrix;     // 16              // 32  (列 0)
                     // 16              // 48  (列 1)
                     // 16              // 64  (列 2)
                     // 16              // 80  (列 3)
    float values[3]; // 4               // 96  (values[0])
                     // 4               // 100 (values[1])
                     // 4               // 104 (values[2])
    bool boolean;    // 4               // 108
    int integer;     // 4               // 112
}; 
```

### 使用
shader端定义：
``` glsl
layout (std430,binding = 1) buffer SSBO_data 
{
    vec3 SSBO_color;
};
```

c++端：
```c++
//数据定义
struct SSBO_Data{
    float color[3];
}ssbo_data;

//创建SSBO
GLuint ssbo;
glGenBuffers(1, &ssbo);
glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec3), NULL, GL_DYNAMIC_DRAW);// 硬编码分配内存
// glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo_data), &ssbo_data, ); // 也是硬编码分配内存
GLuint index = glGetProgramResourceIndex(shader.ID, GL_SHADER_STORAGE_BLOCK, "SSBO_data");
    // 获取block size
    GLint blockSize = 0;
    GLenum prop = GL_BUFFER_DATA_SIZE;
    glGetProgramResourceiv(
        shader.ID, 
        GL_SHADER_STORAGE_BLOCK, 
        index,
        1,      // 属性数量
        &prop,  // 属性数组
        1,      // 返回值数量上限
        NULL,   // 实际返回数量（可选）
        &blockSize // 输出结果
    );
glBufferData(GL_SHADER_STORAGE_BUFFER, blockSize, NULL, GL_DYNAMIC_DRAW); // 申请block大小的空间
// 绑定ssbo绑定点
glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo); // 与着色器中binding值绑定，这里设置是1

// shader buffer块绑定到绑定点
// glShaderStorageBlockBinding(shader.ID, index, 1); (shader中已经指定了，这里注释掉)

/// ssbo获取偏移的api(glGetProgramInterfaceiv + glGetProgramResourceiv)
// 获取该 block 中所有 active 成员的索引列表
//    （因为 SSBO 成员是 "active resources"）
GLint numMembers;
glGetProgramInterfaceiv(
    program,
    GL_BUFFER_VARIABLE,      // ← 查询 buffer 内部的变量（即成员）
    GL_ACTIVE_RESOURCES,
    &numMembers
);

// 3. 遍历每个成员，查询其名称和 offset
std::unordered_map<string, GLint> memberOffset;
for (GLuint i = 0; i < numMembers; ++i) {
    // 获取成员名称（可选）
    char name[256];
    GLsizei length;
    glGetProgramResourceName(
        program,
        GL_BUFFER_VARIABLE,
        i,
        sizeof(name),
        &length,
        name
    );

    // 查询该成员的 offset
    GLenum prop = GL_OFFSET;
    GLint offset = -1;
    glGetProgramResourceiv(
        program,
        GL_BUFFER_VARIABLE,   // ← 关键：查的是 buffer variable（SSBO/UBO 成员）
        i,                    // ← 成员的 resource index（从 0 到 numMembers-1）
        1,
        &prop,
        1,
        NULL,
        &offset
    );
    memberOffset[name] = offset;
    printf("Member '%s' offset = %d\n", name, offset);
}


// 填充：与ubo相同， glBufferSubData或glMapBuffer+memcpy
glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);//先绑定到SSBO对象
vec3 color = vec3(1.0,1.0,0.0);
//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(vec3), &color);//因为是传入值，所以这种方式也可以，注意设置好偏移和数据内存大小就行
GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);//获取着色器中buffer块的内存地址
memcpy(p, &ssbo_data, sizeof(ssbo_data));//将结构体数据拷贝到着色器buffer块地址上
glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);//释放内存地址


// cpu获取ssbo数据
// 1. 内存映射
glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);//隔断作用，为了让数据修改完成
    // GL_BUFFER_UPDATE_BARRIER_BIT	 用于buffer等cpu写入
    // GL_SHADER_STORAGE_BARRIER_BIT	用于cpu等gpu写入
p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY); //获取着色器buffer块内存地址
memcpy(&ssbo_data, p, sizeof(ssbo_data));//拷贝buffer块数据到结构体
glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

// 2. 直接get
glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  
glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, memberOffsets["color"], sizeof(ssbo_data), &ssbo_data); // 直接获取buffer块数据到结构体

```


