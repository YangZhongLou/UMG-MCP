# MCP Server 配置说明

## 环境配置

在启动前，需要配置 Kimi API 密钥：

```bash
# Windows PowerShell
$env:KIMI_API_KEY = "your-api-key"

# 或者写入 .env 文件（项目已忽略，不会提交）
# MCP/.env
KIMI_API_KEY=your-api-key
```

可选环境变量：
| 变量名 | 默认值 | 说明 |
|--------|--------|------|
| `KIMI_API_KEY` | — | **必填**，Kimi API 密钥 |
| `KIMI_MODEL` | `kimi-k2.6` | 模型名称 |
| `KIMI_BASE_URL` | `https://api.moonshot.cn/v1` | API 基础地址 |

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
| `analyze_image` | 分析 UI 设计图，调用 Kimi 多模态 LLM 识别控件结构 | ✅ 可用（需 API Key） |
| `generate_umg` | 根据分析结果生成 UMG 蓝图（通过文件触发 UE5 Editor） | ✅ 可用 |
| `list_supported_widgets` | 列出支持的控件类型 | ✅ 可用 |
