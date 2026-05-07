"""ImageToUMG MCP Server

基于 MCP (Model Context Protocol) 的服务端，提供将 UI 设计图/图片
解析并生成 Unreal Engine UMG 蓝图的工具接口。
"""

import json
from typing import Any

from mcp.server.fastmcp import FastMCP

mcp = FastMCP("ImageToUMG")


@mcp.tool()
async def analyze_image(image_path: str) -> str:
    """分析 UI 设计图，识别控件类型、层级结构与样式信息。

    Args:
        image_path: 本地图片文件的绝对路径

    Returns:
        JSON 字符串，包含解析出的控件树与样式信息
    """
    # TODO: Phase 3 实现图像解析逻辑
    result: dict[str, Any] = {
        "status": "not_implemented",
        "message": "图像解析功能将在 Phase 3 实现",
        "image_path": image_path,
        "widgets": [],
    }
    return json.dumps(result, ensure_ascii=False, indent=2)


@mcp.tool()
async def generate_umg(analysis_json: str, output_path: str = "") -> str:
    """根据 analyze_image 的输出生成 UE5 UMG Widget 蓝图。

    Args:
        analysis_json: analyze_image 返回的 JSON 字符串
        output_path: 蓝图输出目录（可选，默认为项目 Content/UI 目录）

    Returns:
        生成结果描述
    """
    # TODO: Phase 4 实现蓝图生成逻辑
    try:
        analysis = json.loads(analysis_json)
    except json.JSONDecodeError as e:
        return f"解析 analysis_json 失败: {e}"

    result: dict[str, Any] = {
        "status": "not_implemented",
        "message": "UMG 蓝图生成功能将在 Phase 4 实现",
        "analysis_summary": analysis.get("widgets", []),
        "output_path": output_path or "Content/UI/",
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
        {"type": "TextBlock", "priority": "P0", "description": "文本显示"},
        {"type": "Image", "priority": "P0", "description": "图片显示"},
        {"type": "Button", "priority": "P0", "description": "按钮"},
        {"type": "EditableTextBox", "priority": "P1", "description": "输入框"},
        {"type": "ProgressBar", "priority": "P1", "description": "进度条"},
    ]
    return json.dumps(widgets, ensure_ascii=False, indent=2)


if __name__ == "__main__":
    mcp.run(transport="stdio")
