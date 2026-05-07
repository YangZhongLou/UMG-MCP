# Phase 5：样式映射与微调

本阶段目标：建立设计图样式到 UMG 样式的映射规则，并提供人工微调能力。

---

## 任务清单

- [x] 建立样式映射规则（颜色 → SlateColor、字体 → Font 等）
- [x] 支持自定义样式模板（default、dark 模板 + set_style_template 工具）
- [x] 提供生成后的微调接口（UE5 Editor Slate 面板：控件列表 + 属性编辑 + 重新生成）
- [x] 处理边界情况（锚点自动推断：靠左/右/居中/自由定位）

---

## 详细说明

### 建立样式映射规则

- 颜色值 → `FSlateColor`、`FLinearColor`
- 字体信息 → `FSlateFontInfo`（需预置字体资产映射）
- 尺寸 → `FMargin`、`FVector2D`
- 对齐方式 → `EHorizontalAlignment`、`EVerticalAlignment`

### 支持自定义样式模板

- 定义模板配置文件（JSON / YAML）
- 支持按项目预设统一风格

### 提供生成后的微调接口

- 生成报告：列出所有映射决策与置信度
- 标记需人工确认项
- 支持一键重生成

### 处理边界情况

- 屏幕适配（手机 / PC / 平板）
- 动态内容长度（文本溢出、自动换行）
- 复杂布局（绝对定位 vs 流式布局）
