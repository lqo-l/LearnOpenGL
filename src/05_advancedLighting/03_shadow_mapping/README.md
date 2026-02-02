## 定向光的shadow mapping
需要虚拟一个光源位置，使得光源尽可能覆盖场景

步骤：
1. 使用fbo渲染深度贴图
2. debug阴影
3. 使用深度贴图渲染场景及阴影

## 改进阴影失真
1. 深度贴图分辨率有限，会出现自表面阴影现象，需要让阴影贴图深度应用一个偏移让其贴图深度值偏大
2. 越斜面（相对于光照方向）应该偏移越大，所以偏移计算考虑平面法线与光线向量夹角
3. 处理超出深度贴图左右范围的片段：使用GL_CLAMP_TO_BORDER处理深度贴图，让超出范围的采样最大深度，从而得到无shadow值
4. 处理超出光源空间远平面的片段：直接shadow为0，认为没有阴影

### 改进偏移导致的阴影悬浮
渲染深度贴图时，使用正面剔除，避免渲染正表面的深度，这样不使用bias也不会导致自表面阴影（因为深度用的就不是自表面）


## 点光源的阴影
> 虽然050301实现了，但是仅使用向原点的方向进行了一次深度贴图渲染，应该使用cubemaps的全向处理（050302），050301仅做参考。

### 渲染次数
方法一：绑定立方体贴图的不同面到fbo，渲染6次
方法二：`glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);`绑定整个立方体贴图，通过几何着色器扩展6倍顶点一次pass渲染6个面

### 视图矩阵
为每个方向提供一个不同的视图矩阵。用glm::lookAt创建6个观察方向，每个都按顺序注视着立方体贴图的的一个方向：右、左、上、下、近、远：
``` c++
std::vector<glm::mat4> shadowTransforms;
shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3(1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3(0.0,1.0,0.0), glm::vec3(0.0,0.0,1.0)));
shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0)));
shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,-1.0,0.0)));
shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,-1.0,0.0)));
```
up向量：+X、-X、 +Z、-Z 用 (0, -1, 0)，因为OpenGL 立方体贴图的约定为：立方体贴图的 +Y 面纹理坐标原点在**左上角**，为了让渲染的图像正确映射到立方体贴图，需要翻转 Y 轴


注：这里利用外表面法线立方体渲染了一个从内部观察的房间立方体。需要①光照：对法线进行反转②剔除：剔除利用的是顶点定义顺序，与法线无关。所以需要临时关闭剔除
