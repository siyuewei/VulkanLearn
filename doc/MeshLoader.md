# MeshLoader 设计与流程说明

## 1. 目标

当前 MeshLoader 的目标是为工程提供一个稳定、可扩展的网格输入层，满足以下要求：

- 支持 OBJ 文件中的 v、vt、vn 与 f 解析
- 将不同面片格式统一为三角形索引数据
- 输出统一的 MeshAsset 数据结构供渲染层消费
- 在法线缺失时可自动重建法线
- 保留 UV（vt）信息，为后续贴图渲染做准备

MeshLoader 当前聚焦几何读取，不负责材质贴图、骨骼动画、压缩网格等高级能力。

## 2. 模块划分

当前 Mesh 输入模块位于 src/mesh，核心文件如下：

- MeshAsset.h: 定义网格标准数据结构
- ObjMeshLoader.h: 声明 OBJ 读取接口
- ObjMeshLoader.cpp: 实现 OBJ 解析、去重、三角化、法线重建

## 3. 数据结构思路

MeshAsset 是渲染与后续系统共享的中间表示，核心字段：

- positions: 每个顶点的位置
- texcoords: 每个顶点的 UV
- normals: 每个顶点的法线
- indices: 三角形索引（uint32）
- bounds: 轴对齐包围盒（AABB）

关键原则：

- 顶点属性数组长度保持一致（position/uv/normal 对齐）
- MeshAsset 不依赖 Vulkan，保持平台无关
- 由 Loader 负责把 OBJ 的索引系统转换为“可直接渲染”的扁平顶点流 + 索引

## 4. OBJ 解析流程

读取过程按行扫描，按 tag 分流：

1. v 行

- 读取 x y z，写入 rawPositions

2. vt 行

- 读取 u v，写入 rawTexcoords
- 当前实现会做 V 翻转（v = 1 - v），以匹配 Vulkan 常见纹理坐标方向

3. vn 行

- 读取法线，单位化后写入 rawNormals

4. f 行

- 支持以下 token 形态：
  - v
  - v/vt
  - v//vn
  - v/vt/vn
- 每个 token 会解析出 position、texcoord、normal 三个索引
- OBJ 的 1-based 与负索引会统一映射为 0-based 实际索引
- 对每个面执行 fan triangulation，将 n 边形拆分为三角形

## 5. 顶点去重策略

OBJ 的面索引是属性分离索引，不可直接当作渲染顶点索引使用。

当前采用复合键去重：

- Key = (posIndex, texcoordIndex, normalIndex)

原因：

- 同一位置可能对应不同 UV（纹理接缝）
- 同一位置也可能对应不同法线（硬边）

如果只按位置去重，会破坏 UV 接缝和法线分裂，造成渲染错误。

## 6. 法线缺失处理

当 f token 未提供 vn 时：

- 初始法线记为零向量
- 解析完成后按三角面累加面法线
- 最终对每个顶点法线归一化
- 若极端情况下法线长度仍近零，回退到默认 (0,0,1)

这样可以保证即使模型缺少 vn，也能有可用的光照法线数据。

## 7. 错误处理与回退

ObjMeshLoader 采用返回值 + 错误字符串：

- 成功: 返回 true，输出 MeshAsset
- 失败: 返回 false，outError 提供原因

在应用层（VulkanLearnApplication）中：

- 如果 OBJ 加载失败，回退到内置立方体数据
- 保证 Demo 可运行，不因资产错误而崩溃

## 8. 与渲染层的关系

当前渲染顶点结构是 position + color，尚未直接消费 uv。

现阶段流程：

- MeshLoader 读取 position/uv/normal
- 应用层将法线映射为颜色用于可视化
- 索引与顶点上传到 Vulkan Buffer

后续升级贴图时，可以直接把 texcoords 接入 Vertex 与 shader 输入，不需要改 OBJ 读取主流程。

## 9. 设计收益

该方案的优点：

- 读取层与渲染 API 解耦
- 对 OBJ 各种常见面格式兼容性较好
- 支持缺失法线模型自动修复
- 已为贴图渲染预留 UV 数据
- 支持后续扩展更多格式（如 glTF）

## 10. 后续建议

建议按以下顺序演进：

1. 在 Vulkan 顶点结构中加入 uv、normal 并更新 shader
2. 加入 MTL 解析与贴图加载（texture.png）
3. 为物理模块输出网格质量属性（重心、体积近似、惯量初值）
4. 增加 glTF Loader，并继续复用 MeshAsset 中间层

## 11. 常见问题归纳

### Q1: OBJ 不同 token 形态分别是什么意思？

以 f 行中的单个 token 为例，常见形态有：

- v: 仅位置索引
- v/vt: 位置索引 + UV 索引
- v//vn: 位置索引 + 法线索引（中间 vt 缺失）
- v/vt/vn: 位置索引 + UV 索引 + 法线索引

对应的基础数据行：

- v x y z: 顶点位置
- vt u v: 纹理坐标（UV）
- vn nx ny nz: 顶点法线

### Q2: f 行是什么含义？

f 行定义一个面（face），本质是“引用索引的集合”，不是直接写坐标。示例：

- f 1/1/1 2/2/1 3/3/1

表示该三角面的三个角点分别引用：

- 第 1 个位置、第 1 个 UV、第 1 个法线
- 第 2 个位置、第 2 个 UV、第 1 个法线
- 第 3 个位置、第 3 个 UV、第 1 个法线

注意 OBJ 索引是 1-based，读取后需要转换为 0-based。

### Q3: 为什么要做去重，不直接把 raw 数据塞进 mesh？

核心原因：OBJ 是“分离索引”（v、vt、vn 三套索引），而 GPU indexed draw 需要“单一索引”。

Loader 必须把一个角点的 (v, vt, vn) 组合映射到一个统一顶点索引。当前通过复合键实现：

- Key = (posIndex, texcoordIndex, normalIndex)

为什么不是只按位置去重：

- 同一位置可能对应不同 UV（纹理接缝）
- 同一位置可能对应不同法线（硬边）

如果只按位置合并，会造成 UV 拉花、法线错误与光照接缝异常。

### Q4: 一个简化示例中，faceIndices 和 mesh.positions 分别存什么？

示例 OBJ：

```obj
v 0 0 0
v 1 0 0
v 0 1 0
vt 0 0
vt 1 0
vt 0 1
vn 0 0 1
f 1/1/1 2/2/1 3/3/1
```

读取后：

- rawPositions = [(0,0,0), (1,0,0), (0,1,0)]
- rawTexcoords = [(0,1), (1,1), (0,0)]
  - 说明：vt 的 v 分量在 Loader 中做了 1-v 翻转
- rawNormals = [(0,0,1)]

f 行解析后的三个角点组合（0-based）：

- (0,0,0), (1,1,0), (2,2,0)

由于三组 key 都是首次出现，会生成 3 个 mesh 顶点：

- mesh.positions: [(0,0,0), (1,0,0), (0,1,0)]
- mesh.texcoords: [(0,1), (1,1), (0,0)]
- mesh.normals: [(0,0,1), (0,0,1), (0,0,1)]

当前面内索引：

- faceIndices = [0, 1, 2]

追加到总索引后：

- mesh.indices += [0, 1, 2]

结论：

- faceIndices 存的是“当前 f 行在去重后 mesh 顶点流中的索引”
- mesh.positions / texcoords / normals 存的是“已经对齐后的最终顶点属性流”

---

文档版本: v2
更新时间: 2026-03-23
