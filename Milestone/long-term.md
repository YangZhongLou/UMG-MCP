# 长期展望

以下功能不在近期 Roadmap 内，作为项目的长期发展方向记录。

---

## 待探索方向

- [ ] **支持从 Figma / Sketch 等设计文件直接导入**
  - 解析 `.fig`、`.sketch` 等原生格式
  - 获取比截图更精确的图层、样式、组件信息
  - 支持设计系统中的 Symbol / Component 映射到 UMG UserWidget

- [ ] **支持响应式/自适应布局自动生成**
  - 根据设计稿断点生成多分辨率适配蓝图
  - 自动设置 SizeBox、ScaleBox、WidgetSwitcher 等自适应容器

- [ ] **集成更多 UE5 UI 功能（如动画、绑定）**
  - 识别设计稿中的交互动效提示，生成 UMG 动画（Tween / UMG Animation）
  - 生成基础的数据绑定代码框架（MVVM / 属性绑定）

- [ ] **开源 MCP Server 独立部署方案**
  - 提供 Docker 镜像，支持一键部署
  - 支持云端 API（OpenAI、Claude、本地模型）切换
  - 提供 Web Dashboard 管理配置与查看生成历史
