# Phase 4：UMG 蓝图生成

本阶段目标：将解析结果自动转换为 UE5 UMG Widget 蓝图。

---

## 任务清单

- [ ] 开发 UMG Widget 蓝图创建工具
- [ ] 还原控件层级嵌套结构
- [ ] 自动设置控件基础属性（位置、大小、锚点）
- [ ] 支持常用控件（Text、Image、Button、CanvasPanel 等）

---

## 详细说明

### 开发 UMG Widget 蓝图创建工具

- 基于 UE5 Editor Utility / Python 脚本 / C++ 插件实现
- 提供蓝图资产创建 API
- 支持保存到指定 Content 目录

### 还原控件层级嵌套结构

- 根据检测到的空间关系构建 Panel → Child 层级
- 自动选择合适的容器（CanvasPanel、VerticalBox、HorizontalBox 等）

### 自动设置控件基础属性

- 设置 Slot 属性（Position、Size、Anchors）
- 处理 DPI 缩放适配

### 支持常用控件

| 控件类型 | UE5 对应类 | 优先级 |
|----------|-----------|--------|
| 文本 | UTextBlock | P0 |
| 图片 | UImage | P0 |
| 按钮 | UButton | P0 |
| 容器 | UCanvasPanel / UVerticalBox | P0 |
| 输入框 | UEditableTextBox | P1 |
| 进度条 | UProgressBar | P1 |
