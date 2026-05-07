# Phase 2：MCP 通信层

本阶段目标：建立 MCP Server，实现与 AI 客户端的标准化通信。

---

## 任务清单

- [x] 调研并集成 MCP SDK
- [x] 实现 MCP Server 基础服务
- [x] 定义工具接口（Tools）标准
- [x] 完成与 AI 客户端的握手测试

---

## 详细说明

### 调研并集成 MCP SDK

- 调研官方 Python/TypeScript MCP SDK
- 确定 Server 实现语言与框架

### 实现 MCP Server 基础服务

- 搭建 Server 项目结构
- 实现 `initialize` 等基础生命周期方法
- 配置传输层（stdio / SSE）

### 定义工具接口（Tools）标准

- 设计 `analyze_image` 工具参数与返回格式
- 设计 `generate_umg` 工具参数与返回格式
- 编写 JSON Schema 规范

### 完成与 AI 客户端的握手测试

- 在 Claude Desktop / Kimi 等客户端中配置 Server
- 验证工具发现与调用流程
