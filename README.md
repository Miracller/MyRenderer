# MyRender

项目主要是使用C++开发语言（不使用第三方库）实现一个软光栅器，光栅器能读取obj文件、法线贴图、高光贴图、纹理贴图等，并通过渲染管线输出一张几何纹理正确的图。用以学习现代实时图形管线的工作原理。

- MVP变换、视口变换、透视裁剪、背面剔除、光栅化、深度测试等管线操作
- 实现类似vertex shader和fragment shader可编程管线的功能
- 引入Blinn–Phong光照模型增强真实感，shadow map实现硬阴影
- 实现读取切线空间法线贴图

## Result

flat shading 结果

![image](https://user-images.githubusercontent.com/24697586/229333121-b9dbd53b-c9de-4919-8f1e-6ea345d0f1e0.png)

深度图

![image](https://user-images.githubusercontent.com/24697586/229333131-8710a08c-3d91-44b0-83b7-eda6f914a3f4.png)

引入光照和阴影后的结果

![image](https://user-images.githubusercontent.com/24697586/229333100-fbda148f-adf2-4477-a3e4-7382b5279742.png)

其他模型的测试：

![image](https://user-images.githubusercontent.com/24697586/229333673-677109c7-0dfe-4205-90b8-0b490e599dd6.png)


## 技术细节

### 渲染管线的学习：

![image](https://user-images.githubusercontent.com/24697586/229333843-713ee7a8-5dfb-4c2d-bba4-22d20d467dc6.png)

1. Application（CPU端）
    
    主要任务是识别出可视物件，并把他们及材质提交给GPU以供渲染。三大任务：可见性判断、控制着色参数和渲染状态、将rendering primitives 传递到GPU硬件。
    
    还会进行一些软件层面的工作：空间加速算法、视锥剔除、碰撞检测、动画模拟等。
    
    逻辑：执行视锥剔除→ 查询可能需要绘制的图元并生成渲染数据→ 设置渲染状态、绑定shader参数→ 调用drawcall→ 下一阶段
    
2. Geometry Processing
    
    ![image](https://user-images.githubusercontent.com/24697586/229333859-31312f17-8593-4f2f-9fb6-bf3233007d8f.png)
    
    - vertex shading & projection： MVP变换
        
        顶点变换和顶点着色：MVP变换到clip space。
        
        可进行flat shading 和 gouraud shading。
        
    - clipping：由硬件控制，二次视椎剔除，（第一次cpu上，发生在物体层面），第二次发生在包围盒（三角形）层面，剔除不在视椎体中的物体。
    - screen mapping：viewport 变换，[-1,1]^3 到屏幕空间
    
    且projection后可插入 曲面细分(tessellation)、几何着色(geometry shading)。 
    
    tessellation 影响渲染质量，近的话渲染点多，远的话渲染点少。将简单的几何图元（如三角形）细分为更复杂的几何形状。
    
    geometry shading，将图元排序并生成新数据，如粒子系统中烟火的生成。
    
    
    
    3.4.两步：
    
   ![image](https://user-images.githubusercontent.com/24697586/229333865-9b0b39ac-3121-4777-9832-fe8521b2ee66.png)
    
3. Rasterization
    
    此步骤由硬件控制。
    
    - Triangle setup 三角形设置：计算出三角形中的一些重要数据（如三边方程、深度值等），以供遍历阶段使用，这些数据可用于各种插值。
    - Triangle Traversal 三角形遍历：用屏幕上离散的点对三角形可见性函数进行采样。遍历每个像素并判断其是否在三角形内，来决定该像素是否生成fragment。
        
        通过三角形顶点的属性值，插值得到每个像素的属性值。（透视校正差值也在这步执行）
        

4. Pixel Processing
    - Pixel Shading 像素着色：进行光照、阴影等计算，得到屏幕像素的最终颜色值。各种复杂的着色模型、光照计算都是在这个阶段完成的。
    - Merging 测试合并：包括各种测试和混合操作，如 透明测试、模板测试、深度测试以及色彩混合等。经过了测试合并阶段，并存到帧缓冲的像素值，才是最终呈现在屏幕上的图像。
    
--------------------------

### shadow map
1. 关键点是：如果一个点不在阴影内，那么它一定能被光和相机看到（点光源，硬阴影）

2. 两个pass的算法：
- pass 1: 从光源（虚拟相机在光源位置）出发，看向场景，记录一张深度图（深度1）
- pass 2A: 相机在人眼位置，记录各点投射到光源的深度（深度2）
- pass 2B: 比较深度1与深度2，一致说明光源能看到此点；不一致，阴影。

3. 一些问题：浮点精度比较、分辨率、soft shadow(pcss, vssm)。
