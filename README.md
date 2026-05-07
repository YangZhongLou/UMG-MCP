# ImageToUMG

基于 **MCP (Model Context Protocol)** 的自动化工具，用于将 UI 设计图/图片智能转换为 Unreal Engine 的 **UMG (Unreal Motion Graphics)** 界面。

---

## 项目简介

ImageToUMG 打通设计与开发的最后一公里：

1. 上传一张 UI 设计图
2. AI 自动识别控件结构、层级、样式
3. 一键生成 UE5 UMG Widget 蓝图
4. 在编辑器内微调属性后重新生成

**核心优势：** 无需手动拖拽控件、无需反复对齐、一次识别即可产出可用蓝图。

---

## 功能特性

| 功能 | 状态 | 说明 |
|------|------|------|
| 图片解析与识别 | ✅ 已完成 | 调用 Kimi k2.6 多模态 LLM 识别控件类型与布局 |
| 自动生成 UMG 蓝图 | ✅ 已完成 | 根据识别结果生成 `.uasset` 格式的 Widget 蓝图 |
| 控件层级还原 | ✅ 已完成 | 保持设计图中的层级嵌套关系 |
| 基础样式映射 | ✅ 已完成 | 颜色、字体、间距自动映射到 SlateColor/Font |
| MCP 服务端集成 | ✅ 已完成 | 提供标准化的 MCP 工具接口 |
| 样式模板系统 | ✅ 已完成 | 支持 default / dark 等预设模板 |
| 锚点自动推断 | ✅ 已完成 | 根据控件位置自动设置 Anchors |
| 编辑器微调面板 | ✅ 已完成 | UE5 Editor 内 Slate 面板，支持属性修改与重新生成 |

---

## 完整工作流程

```
┌──────────────┐     analyze_image      ┌─────────────┐
│  UI 设计图   │ ─────────────────────> │  Kimi LLM   │
│  (PNG/JPG)   │                        │  多模态解析  │
└──────────────┘                        └──────┬──────┘
                                               │
                                               │ JSON 控件树
                                               ▼
┌──────────────┐     generate_umg       ┌─────────────┐
│   MCP 客户端  │ ─────────────────────> │ MCP Server  │
│(Kimi/Claude) │                        │ (Python)    │
└──────────────┘                        └──────┬──────┘
                                               │ 写入 JSON + trigger
                                               ▼
┌──────────────┐                        ┌─────────────┐
│  UE5 Editor  │ <──── 每 2s 检测 ───── │ 临时文件    │
│  ImageToUMG  │                        │ (Content/)  │
│  Editor 模块  │                        └─────────────┘
└──────┬───────┘
       │ 自动生成 Widget Blueprint
       ▼
┌──────────────┐
│  Slate 面板  │ ──> 微调属性 ──> 重新生成
│  (Window菜单) │
└──────────────┘
```

---

## 环境要求

- **Unreal Engine 5.7**
- **Visual Studio 2022** (Windows) 或 Xcode (macOS)
- **Python 3.13+**
- **uv** (Python 包管理器)
- **Kimi API Key** (或兼容 OpenAI API 的其他多模态模型)
- 支持 MCP 的 AI 客户端（如 Claude Desktop、Kimi Code CLI 等）

---

## 安装步骤

### 1. 克隆仓库并打开 UE5 项目

```bash
git clone https://github.com/YangZhongLou/UMG-MCP.git
cd UMG-MCP
```

双击 `ImageToUMG/ImageToUMG.uproject` 打开，首次打开会提示编译 C++ 项目，选择**是**。

### 2. 配置 MCP Server

```bash
cd MCP

# 复制环境变量模板
copy .env.example .env
# 编辑 .env，填入你的 Kimi API Key

# 安装依赖
uv sync
```

`.env` 文件内容示例：
```
KIMI_API_KEY=sk-your-api-key-here
KIMI_MODEL=kimi-k2.6
```

### 3. 配置 AI 客户端

**Kimi Code CLI** (`~/.kimi/mcp.json`)：
```json
{
  "mcpServers": {
    "image-to-umg": {
      "command": "uv",
      "args": [
        "run",
        "--project",
        "<项目根目录>/MCP",
        "python",
        "main.py"
      ]
    }
  }
}
```

**Claude Desktop** (`claude_desktop_config.json`)：
```json
{
  "mcpServers": {
    "image-to-umg": {
      "command": "uv",
      "args": [
        "run",
        "--project",
        "<项目根目录>/MCP",
        "python",
        "main.py"
      ]
    }
  }
}
```

---

## 使用示例

### 第一步：分析设计图

在 MCP 客户端中调用：
```
请分析这张 UI 设计图：D:/Design/main_menu.png
```

工具 `analyze_image` 会返回 JSON 控件树，包含所有控件的类型、位置、尺寸、文本和样式。

### 第二步：生成蓝图

```
请根据刚才的分析结果生成 UMG 蓝图
```

工具 `generate_umg` 会将 JSON 写入项目 Content 目录，并创建 trigger 文件。确保 **UE5 Editor 处于打开状态**，约 2 秒后蓝图会自动生成。

### 第三步：微调（可选）

在 UE5 Editor 中：
1. 打开菜单 **Window > ImageToUMG 微调面板**
2. 查看生成的控件列表
3. 选中某个控件，修改 X/Y/Width/Height/Text
4. 点击**应用到 JSON**，然后**重新生成蓝图**

---

## 项目结构

```
UMG-MCP/
├── ImageToUMG/                 # Unreal Engine 5.7 项目
│   ├── Content/                # 引擎内容资产
│   │   ├── Blueprints/
│   │   ├── UI/
│   │   └── Materials/
│   ├── Source/
│   │   ├── ImageToUMG/         # 运行时模块
│   │   └── ImageToUMGEditor/   # 编辑器模块
│   │       ├── Private/
│   │       │   ├── ImageToUMGEditor.cpp
│   │       │   ├── ImageToUMGBlueprintGenerator.cpp
│   │       │   └── SImageToUMGModifier.cpp
│   │       └── Public/
│   │           ├── ImageToUMGBlueprintGenerator.h
│   │           └── SImageToUMGModifier.h
│   └── ImageToUMG.uproject
├── MCP/                        # MCP Server (Python)
│   ├── main.py                 # 服务端主程序
│   ├── pyproject.toml
│   ├── CONFIG.md               # 配置说明
│   └── templates/              # 样式模板
│       ├── default.json
│       └── dark.json
├── Milestone/                  # 开发计划文档
│   ├── README.md
│   ├── phase-1.md ~ phase-6.md
│   └── long-term.md
└── README.md
```

---

## 可用工具

| 工具名 | 功能 |
|--------|------|
| `analyze_image` | 分析 UI 设计图，输出 JSON 控件树 |
| `generate_umg` | 将 JSON 控件树提交给 UE5 Editor 生成蓝图 |
| `set_style_template` | 切换样式模板（default / dark） |
| `list_supported_widgets` | 列出支持的 UMG 控件类型 |

---

## 开发计划

详见 [Milestone](./Milestone/) 目录。

| 阶段 | 主题 | 状态 |
|------|------|------|
| Phase 1 | 基础框架搭建 | ✅ 已完成 |
| Phase 2 | MCP 通信层 | ✅ 已完成 |
| Phase 3 | 图片解析与识别 | ✅ 已完成 |
| Phase 4 | UMG 蓝图生成 | ✅ 已完成 |
| Phase 5 | 样式映射与微调 | ✅ 已完成 |
| Phase 6 | 完善与发布 | ✅ 已完成 |

---

## 许可证

MIT License

---

## 相关链接

- [MCP 官方文档](https://modelcontextprotocol.io/)
- [Unreal Engine UMG 文档](https://docs.unrealengine.com/5.0/en-US/umg-ui-designer-in-unreal-engine/)
- [Kimi API 文档](https://platform.kimi.ai/docs)
