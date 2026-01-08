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
当不同shader程序使用相同的uniform变量时,可以使用uniform buffer,只需设置一次,如view和projection.
创建:glGenBuffers
绑定:GL_UNIFORM_BUFFER缓冲目标

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
1. **基准对齐量**:变量占据的空间(含Padding). 
- 标量 base alignment = 4 bytes
- vec2 base alignment = 8 bytes
- vec3 / vec4 base alignment = 16 bytes
- 数组 每个元素的 base alignment 向上取整到 16 bytes
- 矩阵 等价于 列向量数组 每列 base alignment = 16 bytes
vecN 的基准对齐量（base alignment） = sizeof(vec4) = 16 B，只要 N ≥ 2

3. **对齐偏移量**:变量相对于块起始位置的字节偏移量. 
- **必须是基准对齐量的倍数**
- 任意非标量的对齐偏移量为16，如vec2
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
**shared(默认布局):** shared 布局允许驱动自由安排内存布局，需要通过 API 查询 offset。 只能使用`glGetActiveUniformsiv `/`glGetUniformIndices`获取每个uniform变量的偏移量,

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