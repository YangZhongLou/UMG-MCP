// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImageToUMGBlueprintGenerator.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "WidgetBlueprintFactory.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/RichTextBlock.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Widget.h"
#include "Dom/JsonObject.h"
#include "Engine/Font.h"
#include "IAssetTools.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Serialization/JsonSerializer.h"
#include "Styling/SlateColor.h"
#include "UObject/SavePackage.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"

bool FImageToUMGBlueprintGenerator::GenerateFromJson(const FString& JsonFilePath, FString& OutMessage)
{
	// 读取 JSON 文件
	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *JsonFilePath))
	{
		OutMessage = FString::Printf(TEXT("无法读取 JSON 文件: %s"), *JsonFilePath);
		return false;
	}

	// 解析 JSON
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		OutMessage = TEXT("JSON 解析失败");
		return false;
	}

	// 获取输出路径（默认 Content/UI/）
	FString OutputPath = TEXT("/Game/UI");
	if (RootObject->HasField(TEXT("output_path")))
	{
		OutputPath = RootObject->GetStringField(TEXT("output_path"));
	}

	// 获取蓝图名称
	FString BlueprintName = TEXT("BP_GeneratedWidget");
	if (RootObject->HasField(TEXT("blueprint_name")))
	{
		BlueprintName = RootObject->GetStringField(TEXT("blueprint_name"));
	}

	// 创建 Widget Blueprint 工厂
	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->ParentClass = UUserWidget::StaticClass();

	// 创建资产
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UObject* NewAsset = AssetTools.CreateAsset(
		BlueprintName,
		OutputPath,
		UWidgetBlueprint::StaticClass(),
		Factory
	);

	if (!NewAsset)
	{
		OutMessage = TEXT("创建 Widget Blueprint 失败");
		return false;
	}

	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(NewAsset);
	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;

	// 递归创建控件树
	const TArray<TSharedPtr<FJsonValue>>* WidgetsArray = nullptr;
	if (RootObject->TryGetArrayField(TEXT("widgets"), WidgetsArray) && WidgetsArray->Num() > 0)
	{
		for (const TSharedPtr<FJsonValue>& WidgetValue : *WidgetsArray)
		{
			if (TSharedPtr<FJsonObject> WidgetObj = WidgetValue->AsObject())
			{
				CreateWidgetRecursive(WidgetTree, WidgetObj, nullptr);
			}
		}
	}

	// 编译蓝图
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	// 保存资产
	FAssetRegistryModule::AssetCreated(NewAsset);
	UPackage* Package = NewAsset->GetOutermost();
	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		Package->GetName(),
		FPackageName::GetAssetPackageExtension()
	);
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	bool bSaved = UPackage::SavePackage(Package, NewAsset, *PackageFileName, SaveArgs);

	if (bSaved)
	{
		OutMessage = FString::Printf(TEXT("蓝图生成成功: %s/%s"), *OutputPath, *BlueprintName);
	}
	else
	{
		OutMessage = FString::Printf(TEXT("蓝图已创建但保存失败: %s/%s"), *OutputPath, *BlueprintName);
	}

	return bSaved;
}

UWidget* FImageToUMGBlueprintGenerator::CreateWidgetRecursive(
	UWidgetTree* WidgetTree,
	const TSharedPtr<FJsonObject>& JsonObj,
	UWidget* Parent)
{
	if (!JsonObj.IsValid())
	{
		return nullptr;
	}

	FString TypeName;
	if (!JsonObj->TryGetStringField(TEXT("type"), TypeName))
	{
		return nullptr;
	}

	UClass* WidgetClass = GetWidgetClass(TypeName);
	if (!WidgetClass)
	{
		return nullptr;
	}

	FString WidgetName;
	if (!JsonObj->TryGetStringField(TEXT("name"), WidgetName) || WidgetName.IsEmpty())
	{
		WidgetName = FString::Printf(TEXT("Widget_%s"), *TypeName);
	}

	UWidget* NewWidget = WidgetTree->ConstructWidget(TSubclassOf<UWidget>(WidgetClass), FName(*WidgetName));
	if (!NewWidget)
	{
		return nullptr;
	}

	// 设置通用属性
	SetupCommonProperties(NewWidget, JsonObj);

	// 添加到父控件
	if (Parent)
	{
		if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(Parent))
		{
			Canvas->AddChild(NewWidget);
			SetupCanvasSlot(NewWidget, JsonObj);
		}
		else if (UVerticalBox* VBox = Cast<UVerticalBox>(Parent))
		{
			VBox->AddChild(NewWidget);
		}
		else if (UHorizontalBox* HBox = Cast<UHorizontalBox>(Parent))
		{
			HBox->AddChild(NewWidget);
		}
		else if (UOverlay* Overlay = Cast<UOverlay>(Parent))
		{
			Overlay->AddChild(NewWidget);
		}
		else if (UButton* Button = Cast<UButton>(Parent))
		{
			Button->AddChild(NewWidget);
		}
	}
	else
	{
		// 没有父控件时作为 RootWidget（通常是 CanvasPanel）
		WidgetTree->RootWidget = NewWidget;
	}

	// 递归创建子控件
	const TArray<TSharedPtr<FJsonValue>>* ChildrenArray = nullptr;
	if (JsonObj->TryGetArrayField(TEXT("children"), ChildrenArray))
	{
		for (const TSharedPtr<FJsonValue>& ChildValue : *ChildrenArray)
		{
			if (TSharedPtr<FJsonObject> ChildObj = ChildValue->AsObject())
			{
				CreateWidgetRecursive(WidgetTree, ChildObj, NewWidget);
			}
		}
	}

	return NewWidget;
}

void FImageToUMGBlueprintGenerator::SetupCanvasSlot(UWidget* Widget, const TSharedPtr<FJsonObject>& JsonObj)
{
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot);
	if (!CanvasSlot)
	{
		return;
	}

	double X = 0.0, Y = 0.0, Width = 100.0, Height = 40.0;
	JsonObj->TryGetNumberField(TEXT("x"), X);
	JsonObj->TryGetNumberField(TEXT("y"), Y);
	JsonObj->TryGetNumberField(TEXT("width"), Width);
	JsonObj->TryGetNumberField(TEXT("height"), Height);

	CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	CanvasSlot->SetPosition(FVector2D(static_cast<float>(X), static_cast<float>(Y)));
	CanvasSlot->SetSize(FVector2D(static_cast<float>(Width), static_cast<float>(Height)));
}

void FImageToUMGBlueprintGenerator::SetupCommonProperties(UWidget* Widget, const TSharedPtr<FJsonObject>& JsonObj)
{
	// 文本内容
	FString TextContent;
	if (JsonObj->TryGetStringField(TEXT("text"), TextContent))
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			TextBlock->SetText(FText::FromString(TextContent));
		}
		else if (UEditableTextBox* EditBox = Cast<UEditableTextBox>(Widget))
		{
			EditBox->SetText(FText::FromString(TextContent));
		}
		else if (UMultiLineEditableTextBox* MultiEdit = Cast<UMultiLineEditableTextBox>(Widget))
		{
			MultiEdit->SetText(FText::FromString(TextContent));
		}
		else if (UButton* Button = Cast<UButton>(Widget))
		{
			// 按钮默认没有文本，需要添加一个 TextBlock 子控件
			// 这里不做处理，由子控件 children 来处理
		}
	}

	// 样式解析
	const TSharedPtr<FJsonObject>* StyleObj = nullptr;
	if (JsonObj->TryGetObjectField(TEXT("style"), StyleObj))
	{
		// 文本颜色
		FString TextColorHex;
		if ((*StyleObj)->TryGetStringField(TEXT("text_color"), TextColorHex))
		{
			FLinearColor Color = ParseHexColor(TextColorHex);
			if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
			{
				TextBlock->SetColorAndOpacity(FSlateColor(Color));
			}
		}

		// 背景颜色（按钮）
		FString BgColorHex;
		if ((*StyleObj)->TryGetStringField(TEXT("background_color"), BgColorHex))
		{
			FLinearColor Color = ParseHexColor(BgColorHex);
			if (UButton* Button = Cast<UButton>(Widget))
			{
				FButtonStyle ButtonStyle = Button->GetStyle();
				ButtonStyle.Normal.TintColor = FSlateColor(Color);
				Button->SetStyle(ButtonStyle);
			}
		}

		// 字体大小
		double FontSize = 16.0;
		if ((*StyleObj)->TryGetNumberField(TEXT("font_size"), FontSize))
		{
			if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
			{
				FSlateFontInfo FontInfo = TextBlock->GetFont();
				FontInfo.Size = static_cast<int32>(FontSize);
				TextBlock->SetFont(FontInfo);
			}
		}
	}
}

UClass* FImageToUMGBlueprintGenerator::GetWidgetClass(const FString& TypeName)
{
	static TMap<FString, UClass*> WidgetClassMap;
	if (WidgetClassMap.Num() == 0)
	{
		WidgetClassMap.Add(TEXT("CanvasPanel"), UCanvasPanel::StaticClass());
		WidgetClassMap.Add(TEXT("VerticalBox"), UVerticalBox::StaticClass());
		WidgetClassMap.Add(TEXT("HorizontalBox"), UHorizontalBox::StaticClass());
		WidgetClassMap.Add(TEXT("Overlay"), UOverlay::StaticClass());
		WidgetClassMap.Add(TEXT("TextBlock"), UTextBlock::StaticClass());
		WidgetClassMap.Add(TEXT("RichTextBlock"), URichTextBlock::StaticClass());
		WidgetClassMap.Add(TEXT("Image"), UImage::StaticClass());
		WidgetClassMap.Add(TEXT("Button"), UButton::StaticClass());
		WidgetClassMap.Add(TEXT("EditableTextBox"), UEditableTextBox::StaticClass());
		WidgetClassMap.Add(TEXT("MultiLineEditableTextBox"), UMultiLineEditableTextBox::StaticClass());
		WidgetClassMap.Add(TEXT("ProgressBar"), UProgressBar::StaticClass());
		WidgetClassMap.Add(TEXT("Slider"), USlider::StaticClass());
		WidgetClassMap.Add(TEXT("CheckBox"), UCheckBox::StaticClass());
	}

	UClass** FoundClass = WidgetClassMap.Find(TypeName);
	return FoundClass ? *FoundClass : nullptr;
}

FLinearColor FImageToUMGBlueprintGenerator::ParseHexColor(const FString& HexColor)
{
	FString CleanHex = HexColor.Replace(TEXT("#"), TEXT(""));
	if (CleanHex.Len() == 6)
	{
		uint32 Value = FParse::HexNumber(*CleanHex);
		float R = static_cast<float>((Value >> 16) & 0xFF) / 255.0f;
		float G = static_cast<float>((Value >> 8) & 0xFF) / 255.0f;
		float B = static_cast<float>(Value & 0xFF) / 255.0f;
		return FLinearColor(R, G, B, 1.0f);
	}
	return FLinearColor::White;
}
