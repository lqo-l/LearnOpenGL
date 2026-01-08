`glBufferData(target, size, data, usage);`函数
## usage参数作用
usage 是一个提示（hint），告诉 OpenGL 驱动程序你打算如何使用这块缓冲区数据，以便驱动可以做出更优的内存分配和管理策略（比如放在 GPU 显存、系统内存，是否启用缓存、双缓冲等）。但它不是强制约束——你仍然可以在声明为 GL_STATIC_DRAW 的缓冲区上调用 glBufferSubData，只是性能可能不如预期。

## usage选项
`GL_STATIC_DRAW`:静态（很少或从不更改）	绘制（由应用程序提供，GPU 读取用于渲染）	
`GL_DYNAMIC_DRAW`: 动态（频繁更改）	    绘制	
`GL_STREAM_DRAW`: 流式（每帧都换新数据）	绘制	

> 此外，还有两类后缀变体:`_READ`(从GPU读回数据),`_COPY`(GPU内部复制)

对于STATIC： 驱动倾向于将其放入GPU 专属显存（VRAM），以获得最快的渲染读取速度。
对于DYNAMIC： 适用于多次更新、多次使用的数据（如每帧更新的骨骼动画、粒子位置、view/proj 矩阵 UBO 等）。驱动可能会选择可写映射的 GPU 内存或系统内存+高效上传路径，并可能内部实现双缓冲以避免同步卡顿。
对于STREAM：适用于一次性使用的数据（如调试线框、临时几何体）。驱动可能会分配临时上传内存（如 AGP/system memory），上传后很快释放或重用。再次使用需要重新上传数据 `glBufferData`