## buffer数据填充方式

```c++
    // ...
    glBindBuffer(GL_ARRAY_BUFFER,vbo);

    // 1. 使用 glBufferData 一次性分配和初始化缓冲区数据
    // glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxCubeVertices), &skyboxCubeVertices, GL_STATIC_DRAW);
    
    // 2. 使用 glBufferData 分配内存，然后使用 glBufferSubData 复制数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxCubeVertices), nullptr, GL_STATIC_DRAW); // 先分配内存但不复制数据,漏了程序会闪退
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skyboxCubeVertices), &skyboxCubeVertices); // 范围[0,0+size]

    // 3. glMapBuffer方式，映射内存后直接复制数据，适合文件读取数据
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxCubeVertices), nullptr, GL_STATIC_DRAW); // 先分配内存但不复制数据,漏了程序会闪退
    void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(ptr, skyboxCubeVertices, sizeof(skyboxCubeVertices));
    glUnmapBuffer(GL_ARRAY_BUFFER);
```

## 分批顶点属性的填充
交错属性：123123123123, 一个大数组
分批属性：1111  2222  3333， 分开的各属性独立的数组
```c++
float positions[] = { ... };
float normals[] = { ... };
float tex[] = { ... };
// 填充缓冲
glBufferSubData(GL_ARRAY_BUFFER, 0,                                 sizeof(positions),  &positions);
glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions),                 sizeof(normals),    &normals);
glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions)+sizeof(normals), sizeof(tex),        &tex);
// 设置顶点属性
// 形参：index, size, type, normalized, stride, pointer
glVertexAttribPointer(0, 3, float, false, 3*sizeof(float), 0); // stride代表同组属性之间的内存字节间隔
glVertexAttribPointer(1, 3, float, false, 3*sizeof(float), (void*)(sizeof(positions))); // pointer: 当前绑定到 GL_ARRAY_BUFFER 的那块 VBO 内部的字节偏移量.从这里开始拿数据。
glVertexAttribPointer(1, 2, float, false, 2*sizeof(float), (void*)(sizeof(positions)+sizeof(normals)));
```

## 复制缓冲
```c++
// 原型
void glCopyBufferSubData(GLenum readtarget, GLenum writetarget, GLintptr readoffset,
                         GLintptr writeoffset, GLsizeiptr size);
```

无法将两个缓冲绑定到同一个缓冲目标,如同时绑定`GL_ARRAY_BUFFER`,可以使用这两种缓冲目标:`GL_COPY_READ_BUFFER`和`GL_COPY_WRITE_BUFFER`
```c++
glBindBuffer(GL_COPY_READ_BUFFER, vbo1); // 其中一个其实也可以正常绑定到GL_ARRAY_BUFFER
glBindBuffer(GL_COPY_WRITE_BUFFER, vbo2);
glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertexData));
```