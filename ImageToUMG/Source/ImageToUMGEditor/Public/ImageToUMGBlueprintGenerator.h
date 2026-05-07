// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Layout/Anchors.h"

class UWidget;
class UWidgetTree;
class UClass;

class FImageToUMGBlueprintGenerator
{
public:
	/** 从 JSON 文件生成 Widget Blueprint */
	static bool GenerateFromJson(const FString& JsonFilePath, FString& OutMessage);

private:
	/** 画布尺寸，用于锚点推断 */
	static float CanvasWidth;
	static float CanvasHeight;

	/** 递归创建控件 */
	static UWidget* CreateWidgetRecursive(UWidgetTree* WidgetTree, const TSharedPtr<FJsonObject>& JsonObj, UWidget* Parent);

	/** 为 CanvasPanel 子控件设置 Slot */
	static void SetupCanvasSlot(UWidget* Widget, const TSharedPtr<FJsonObject>& JsonObj);

	/** 推断并设置锚点 */
	static FAnchors InferAnchors(float X, float Y, float Width, float Height);

	/** 设置通用属性（文本、颜色等） */
	static void SetupCommonProperties(UWidget* Widget, const TSharedPtr<FJsonObject>& JsonObj);

	/** 根据类型名获取 UMG 控件类 */
	static UClass* GetWidgetClass(const FString& TypeName);

	/** 解析 HEX 颜色 */
	static FLinearColor ParseHexColor(const FString& HexColor);
};
