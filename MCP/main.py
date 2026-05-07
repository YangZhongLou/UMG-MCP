"""ImageToUMG MCP Server

基于 MCP (Model Context Protocol) 的服务端，提供将 UI 设计图/图片
解析并生成 Unreal Engine UMG 蓝图的工具接口。

依赖环境变量:
    KIMI_API_KEY: Kimi API 密钥（必需）
    KIMI_MODEL: 模型名称，默认 kimi-k2.6
    KIMI_BASE_URL: API 基础地址，默认 https://api.moonshot.cn/v1
"""

import base64
import json
import os
from pathlib import Path
from typing import Any

from mcp.server.fastmcp import FastMCP
from openai import AsyncOpenAI

mcp = FastMCP("ImageToUMG")

# ---------- LLM Client ----------

_client: AsyncOpenAI | None = None


def _get_client() -> AsyncOpenAI:
    global _client
    if _client is None:
        api_key = os.environ.get("KIMI_API_KEY", "")
        base_url = os.environ.get("KIMI_BASE_URL", "https://api.moonshot.cn/v1")
        if not api_key:
            raise RuntimeError(
                "KIMI_API_KEY 环境变量未设置，请在 .env 文件或系统环境中配置 API 密钥"
            )
        _client = AsyncOpenAI(api_key=api_key, base_url=base_url)
    return _client


def _get_model() -> str:
    return os.environ.get("KIMI_MODEL", "kimi-k2.6")


# ---------- Image Utilities ----------


def _image_to_base64(image_path: str) -> str:
    path = Path(image_path)
    if not path.exists():
        raise FileNotFoundError(f"图片文件不存在: {image_path}")
    with path.open("rb") as f:
        data = f.read()
    mime = "image/png"
    suffix = path.suffix.lower()
    if suffix in (".jpg", ".jpeg"):
        mime = "image/jpeg"
    elif suffix == ".webp":
        mime = "image/webp"
    elif suffix == ".gif":
        mime = "image/gif"
    return f"data:{mime};base64,{base64.b64encode(data).decode()}"


# ---------- Prompts ----------

_ANALYZE_PROMPT = """你是一位专业的 UI 设计师和 UE5 UMG 开发专家。请仔细分析这张 UI 设计图，输出一份详细的控件结构分析报告。

要求：
1. 识别所有可见的 UI 控件（按钮、文本、图片、输入框、进度条、滑动条等）
2. 输出每个控件的类型、位置、尺寸、文本内容和样式信息
3. 推断控件之间的层级嵌套关系
4. 所有坐标和尺寸以像素为单位，基于图片原始尺寸
5. 输出必须为合法的 JSON 格式，不要包含 markdown 代码块标记，直接输出 JSON 文本

JSON 结构示例：
{
  "canvas_width": 1920,
  "canvas_height": 1080,
  "widgets": [
    {
      "type": "CanvasPanel",
      "name": "RootCanvas",
      "x": 0,
      "y": 0,
      "width": 1920,
      "height": 1080,
      "text": "",
      "style": {},
      "children": [
        {
          "type": "Button",
          "name": "BtnStart",
          "x": 860,
          "y": 500,
          "width": 200,
          "height": 60,
          "text": "开始游戏",
          "style": {"background_color": "#4CAF50", "font_size": 24, "text_color": "#FFFFFF"},
          "children": []
        }
      ]
    }
  ]
}

支持的控件类型（请严格使用以下类型名）：
- CanvasPanel（根容器，通常作为顶层）
- VerticalBox（垂直自动布局容器）
- HorizontalBox（水平自动布局容器）
- Overlay（叠层容器，控件重叠时使用）
- TextBlock（纯文本显示）
- RichTextBlock（富文本）
- Image（图片/图标）
- Button（按钮）
- EditableTextBox（单行输入框）
- MultiLineEditableTextBox（多行输入框）
- ProgressBar（进度条）
- Slider（滑动条）
- CheckBox（复选框）

注意事项：
- 如果无法确定精确坐标，请给出合理估计，不要留空
- 颜色使用 HEX 格式（如 #FF5733）
- 字体大小以整数 pt 为单位估算
- 层级关系应尽量准确反映视觉上的嵌套，优先使用 CanvasPanel 作为根节点
- 对于纯装饰性背景，也作为一个 Image 控件输出
- 只输出 JSON，不要有任何额外解释文字
"""


# ---------- MCP Tools ----------


@mcp.tool()
async def analyze_image(image_path: str) -> str:
    """分析 UI 设计图，识别控件类型、层级结构与样式信息。

    Args:
        image_path: 本地图片文件的绝对路径

    Returns:
        JSON 字符串，包含解析出的控件树与样式信息
    """
    try:
        image_b64 = _image_to_base64(image_path)
    except FileNotFoundError as e:
        return json.dumps({"status": "error", "message": str(e)}, ensure_ascii=False)
    except Exception as e:
        return json.dumps(
            {"status": "error", "message": f"读取图片失败: {e}"}, ensure_ascii=False
        )

    try:
        client = _get_client()
    except RuntimeError as e:
        return json.dumps({"status": "error", "message": str(e)}, ensure_ascii=False)

    try:
        response = await client.chat.completions.create(
            model=_get_model(),
            messages=[
                {
                    "role": "user",
                    "content": [
                        {"type": "text", "text": _ANALYZE_PROMPT},
                        {
                            "type": "image_url",
                            "image_url": {"url": image_b64},
                        },
                    ],
                }
            ],
            temperature=0.2,
            max_tokens=4096,
        )
        raw_content = response.choices[0].message.content or "{}"

        # 清理可能的 markdown 代码块
        content = raw_content.strip()
        if content.startswith("```json"):
            content = content[7:]
        if content.startswith("```"):
            content = content[3:]
        if content.endswith("```"):
            content = content[:-3]
        content = content.strip()

        # 验证 JSON
        parsed = json.loads(content)
        parsed["status"] = "success"
        parsed["message"] = "图像分析完成"
        return json.dumps(parsed, ensure_ascii=False, indent=2)

    except json.JSONDecodeError as e:
        return json.dumps(
            {
                "status": "error",
                "message": f"LLM 返回的内容不是合法 JSON: {e}",
                "raw_response": raw_content[:500] if "raw_content" in dir() else "",
            },
            ensure_ascii=False,
        )
    except Exception as e:
        return json.dumps(
            {"status": "error", "message": f"调用 LLM API 失败: {e}"},
            ensure_ascii=False,
        )


@mcp.tool()
async def generate_umg(analysis_json: str, output_path: str = "") -> str:
    """根据 analyze_image 的输出生成 UE5 UMG Widget 蓝图。

    工作原理：
    1. 将 analysis_json 写入项目 Content 目录的临时 JSON 文件
    2. 写入 trigger 文件通知 UE5 Editor 执行生成
    3. UE5 Editor 检测到 trigger 后自动创建并保存 Widget Blueprint

    Args:
        analysis_json: analyze_image 返回的 JSON 字符串
        output_path: 蓝图输出路径（可选，如 /Game/UI）

    Returns:
        生成请求状态描述
    """
    try:
        analysis = json.loads(analysis_json)
    except json.JSONDecodeError as e:
        return json.dumps(
            {"status": "error", "message": f"解析 analysis_json 失败: {e}"},
            ensure_ascii=False,
        )

    if analysis.get("status") != "success":
        return json.dumps(
            {
                "status": "error",
                "message": f"分析结果状态异常: {analysis.get('message', 'unknown')}",
            },
            ensure_ascii=False,
        )

    # 计算项目路径
    mcp_dir = Path(__file__).parent.resolve()
    project_root = mcp_dir.parent
    content_dir = project_root / "ImageToUMG" / "Content"
    content_dir.mkdir(parents=True, exist_ok=True)

    json_file = content_dir / "ImageToUMG_WidgetGen.json"
    trigger_file = content_dir / "ImageToUMG_Trigger.tmp"

    # 补充输出路径和蓝图名称到 JSON
    analysis["output_path"] = output_path or "/Game/UI"
    if "blueprint_name" not in analysis:
        analysis["blueprint_name"] = "BP_GeneratedWidget"

    try:
        with json_file.open("w", encoding="utf-8") as f:
            json.dump(analysis, f, ensure_ascii=False, indent=2)

        # 写入 trigger 文件通知 UE5 Editor
        trigger_file.write_text("", encoding="utf-8")
    except Exception as e:
        return json.dumps(
            {"status": "error", "message": f"写入文件失败: {e}"},
            ensure_ascii=False,
        )

    result: dict[str, Any] = {
        "status": "queued",
        "message": "生成请求已提交到 UE5 Editor，请确保编辑器处于打开状态",
        "json_file": str(json_file),
        "trigger_file": str(trigger_file),
        "widgets_count": _count_widgets(analysis.get("widgets", [])),
        "output_path": analysis["output_path"],
        "blueprint_name": analysis["blueprint_name"],
    }
    return json.dumps(result, ensure_ascii=False, indent=2)


@mcp.tool()
async def list_supported_widgets() -> str:
    """列出当前支持的 UMG 控件类型。

    Returns:
        JSON 字符串，包含支持的控件列表
    """
    widgets = [
        {"type": "CanvasPanel", "priority": "P0", "description": "根容器"},
        {"type": "VerticalBox", "priority": "P0", "description": "垂直自动布局"},
        {"type": "HorizontalBox", "priority": "P0", "description": "水平自动布局"},
        {"type": "Overlay", "priority": "P0", "description": "叠层容器"},
        {"type": "TextBlock", "priority": "P0", "description": "文本显示"},
        {"type": "RichTextBlock", "priority": "P1", "description": "富文本"},
        {"type": "Image", "priority": "P0", "description": "图片/图标"},
        {"type": "Button", "priority": "P0", "description": "按钮"},
        {"type": "EditableTextBox", "priority": "P1", "description": "单行输入框"},
        {"type": "MultiLineEditableTextBox", "priority": "P1", "description": "多行输入框"},
        {"type": "ProgressBar", "priority": "P1", "description": "进度条"},
        {"type": "Slider", "priority": "P1", "description": "滑动条"},
        {"type": "CheckBox", "priority": "P1", "description": "复选框"},
    ]
    return json.dumps(widgets, ensure_ascii=False, indent=2)


# ---------- Helpers ----------


def _count_widgets(widgets: list[dict[str, Any]]) -> int:
    count = 0
    for w in widgets:
        count += 1
        count += _count_widgets(w.get("children", []))
    return count


if __name__ == "__main__":
    mcp.run(transport="stdio")
