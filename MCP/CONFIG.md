# MCP Server 配置说明

## 启动方式

```bash
cd MCP
uv run python main.py
```

## Kimi Code CLI 配置

在 Kimi Code CLI 的 MCP 配置文件中添加（通常是 `~/.kimi/mcp.json`）：

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

## Claude Desktop 配置

在 `claude_desktop_config.json` 中添加：

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

## 可用工具

| 工具名 | 功能 | 状态 |
|--------|------|------|
| `analyze_image` | 分析 UI 设计图，识别控件结构 | 🚧 占位实现 |
| `generate_umg` | 根据分析结果生成 UMG 蓝图 | 🚧 占位实现 |
| `list_supported_widgets` | 列出支持的控件类型 | ✅ 可用 |
