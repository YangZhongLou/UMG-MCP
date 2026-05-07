# ImageToUMG

基于 **MCP (Model Context Protocol)** 的自动化工具，用于将 UI 设计图/图片智能转换为 Unreal Engine 的 **UMG (Unreal Motion Graphics)** 界面。

---

## 项目简介

ImageToUMG 旨在打通设计与开发的最后一公里。通过 MCP 协议与 AI 模型交互，项目能够解析输入的 UI 设计图或截图，自动生成对应的 UMG Widget 蓝图结构、控件层级以及基础样式配置，大幅减少手动搭建 UI 的重复劳动。

---

## 功能特性

| 功能 | 状态 | 说明 |
|------|------|------|
| 图片解析与识别 | 🚧 开发中 | 解析 UI 设计图，识别控件类型与布局 |
| 自动生成 UMG 蓝图 | 🚧 开发中 | 根据识别结果生成 `.uasset` 格式的 Widget 蓝图 |
| 控件层级还原 | 🚧 开发中 | 保持设计图中的层级嵌套关系 |
| 基础样式映射 | 📋 规划中 | 颜色、字体、间距等样式自动匹配 |
| MCP 服务端集成 | 🚧 开发中 | 提供标准化的 MCP 工具接口 |

---

## 技术栈

- **Unreal Engine 5.7** — 目标运行引擎
- **MCP (Model Context Protocol)** — AI 与引擎间的通信协议
- **C++ / Blueprint** — 引擎端开发语言
- **Python** — MCP Server 端实现（规划中）

---

## 项目结构

```
UMG-MCP/
├── ImageToUMG/                 # Unreal Engine 项目目录
│   ├── Content/                # 引擎内容资产（蓝图、材质、UI等）
│   ├── Source/
│   │   └── ImageToUMG/         # C++ 源码模块
│   │       ├── ImageToUMG.cpp  # 游戏模块入口
│   │       └── ImageToUMG.h
│   ├── Config/                 # 引擎配置文件
│   └── ImageToUMG.uproject     # UE5 项目文件
└── README.md
```

---

## 快速开始

### 环境要求

- Unreal Engine 5.7
- Visual Studio 2022 (Windows) 或 Xcode (macOS)
- （可选）支持 MCP 的 AI 客户端（如 Claude Desktop、Kimi 等）

### 打开项目

1. 克隆本仓库
2. 双击 `ImageToUMG/ImageToUMG.uproject` 或在 Epic Games Launcher 中打开
3. 如提示编译 C++ 项目，选择 **是**

### 使用 MCP 功能（规划中）

```
# 待补充：MCP Server 的启动与配置方式
```

---

## 开发计划

详见 [Milestone](./Milestone/) 目录。

---

## 许可证

待定

---

## 相关链接

- [MCP 官方文档](https://modelcontextprotocol.io/)
- [Unreal Engine UMG 文档](https://docs.unrealengine.com/5.0/en-US/umg-ui-designer-in-unreal-engine/)
