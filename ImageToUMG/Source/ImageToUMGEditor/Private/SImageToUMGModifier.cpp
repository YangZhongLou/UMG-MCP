// Copyright Epic Games, Inc. All Rights Reserved.

#include "SImageToUMGModifier.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "EditorStyleSet.h"
#include "HAL/FileManager.h"
#include "ImageToUMGBlueprintGenerator.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

void SImageToUMGModifier::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("ImageToUMG 微调面板")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("刷新")))
				.OnClicked(this, &SImageToUMGModifier::OnRegenerateClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8, 4)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(FText::FromString(TEXT("点击刷新读取最新生成结果")))
			.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
		]

		// 控件列表
		+ SVerticalBox::Slot()
		.FillHeight(0.4f)
		.Padding(8)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SAssignNew(ListView, SListView<TSharedPtr<FWidgetInfo>>)
				.ListItemsSource(&WidgetList)
				.OnGenerateRow(this, &SImageToUMGModifier::OnGenerateRow)
				.OnSelectionChanged(this, &SImageToUMGModifier::OnWidgetSelected)
			]
		]

		// 属性编辑区
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(8)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 4)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("选中控件属性")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("X:")))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SAssignNew(XTextBox, SEditableTextBox)
						.IsEnabled(false)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Y:")))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SAssignNew(YTextBox, SEditableTextBox)
						.IsEnabled(false)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Width:")))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SAssignNew(WidthTextBox, SEditableTextBox)
						.IsEnabled(false)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Height:")))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SAssignNew(HeightTextBox, SEditableTextBox)
						.IsEnabled(false)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Text:")))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						SAssignNew(TextTextBox, SEditableTextBox)
						.IsEnabled(false)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 8)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("应用到 JSON")))
						.OnClicked(this, &SImageToUMGModifier::OnApplyClicked)
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("重新生成蓝图")))
						.OnClicked(this, &SImageToUMGModifier::OnRegenerateClicked)
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("打开蓝图")))
						.OnClicked(this, &SImageToUMGModifier::OnOpenBlueprintClicked)
					]
				]
			]
		]
	];

	RefreshFromJson();
}

void SImageToUMGModifier::RefreshFromJson()
{
	JsonFilePath = FPaths::ProjectContentDir() / TEXT("ImageToUMG_WidgetGen.json");
	WidgetList.Empty();
	SelectedWidget.Reset();

	if (!FPaths::FileExists(JsonFilePath))
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(FText::FromString(TEXT("未找到生成记录，请先执行 MCP generate_umg")));
		}
		if (ListView.IsValid())
		{
			ListView->RequestListRefresh();
		}
		return;
	}

	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *JsonFilePath))
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(FText::FromString(TEXT("读取 JSON 文件失败")));
		}
		return;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
	if (!FJsonSerializer::Deserialize(Reader, RootJsonObject) || !RootJsonObject.IsValid())
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(FText::FromString(TEXT("JSON 解析失败")));
		}
		return;
	}

	// 递归解析控件
	const TArray<TSharedPtr<FJsonValue>>* WidgetsArray = nullptr;
	if (RootJsonObject->TryGetArrayField(TEXT("widgets"), WidgetsArray))
	{
		for (const TSharedPtr<FJsonValue>& Val : *WidgetsArray)
		{
			if (TSharedPtr<FJsonObject> Obj = Val->AsObject())
			{
				ParseWidgetsRecursive(Obj);
			}
		}
	}

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(
			FString::Printf(TEXT("共 %d 个控件，JSON: %s"), WidgetList.Num(), *JsonFilePath)
		));
	}

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

void SImageToUMGModifier::ParseWidgetsRecursive(const TSharedPtr<FJsonObject>& JsonObj)
{
	TSharedPtr<FWidgetInfo> Info = MakeShared<FWidgetInfo>();
	Info->JsonObject = JsonObj;
	JsonObj->TryGetStringField(TEXT("name"), Info->Name);
	JsonObj->TryGetStringField(TEXT("type"), Info->Type);
	JsonObj->TryGetNumberField(TEXT("x"), Info->X);
	JsonObj->TryGetNumberField(TEXT("y"), Info->Y);
	JsonObj->TryGetNumberField(TEXT("width"), Info->Width);
	JsonObj->TryGetNumberField(TEXT("height"), Info->Height);
	JsonObj->TryGetStringField(TEXT("text"), Info->Text);
	WidgetList.Add(Info);

	const TArray<TSharedPtr<FJsonValue>>* Children = nullptr;
	if (JsonObj->TryGetArrayField(TEXT("children"), Children))
	{
		for (const TSharedPtr<FJsonValue>& Val : *Children)
		{
			if (TSharedPtr<FJsonObject> ChildObj = Val->AsObject())
			{
				ParseWidgetsRecursive(ChildObj);
			}
		}
	}
}

TSharedRef<ITableRow> SImageToUMGModifier::OnGenerateRow(
	TSharedPtr<FWidgetInfo> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FWidgetInfo>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.4f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Type))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.6f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Name))
			.ColorAndOpacity(FLinearColor::Gray)
		]
	];
}

void SImageToUMGModifier::OnWidgetSelected(TSharedPtr<FWidgetInfo> Item, ESelectInfo::Type SelectInfo)
{
	SelectedWidget = Item;
	bool bEnabled = Item.IsValid();

	if (XTextBox.IsValid())
	{
		XTextBox->SetEnabled(bEnabled);
		XTextBox->SetText(bEnabled ? FText::FromString(FString::Printf(TEXT("%.0f"), Item->X)) : FText::GetEmpty());
	}
	if (YTextBox.IsValid())
	{
		YTextBox->SetEnabled(bEnabled);
		YTextBox->SetText(bEnabled ? FText::FromString(FString::Printf(TEXT("%.0f"), Item->Y)) : FText::GetEmpty());
	}
	if (WidthTextBox.IsValid())
	{
		WidthTextBox->SetEnabled(bEnabled);
		WidthTextBox->SetText(bEnabled ? FText::FromString(FString::Printf(TEXT("%.0f"), Item->Width)) : FText::GetEmpty());
	}
	if (HeightTextBox.IsValid())
	{
		HeightTextBox->SetEnabled(bEnabled);
		HeightTextBox->SetText(bEnabled ? FText::FromString(FString::Printf(TEXT("%.0f"), Item->Height)) : FText::GetEmpty());
	}
	if (TextTextBox.IsValid())
	{
		TextTextBox->SetEnabled(bEnabled);
		TextTextBox->SetText(bEnabled ? FText::FromString(Item->Text) : FText::GetEmpty());
	}
}

FReply SImageToUMGModifier::OnApplyClicked()
{
	if (!SelectedWidget.IsValid() || !SelectedWidget->JsonObject.IsValid())
	{
		return FReply::Handled();
	}

	SelectedWidget->JsonObject->SetNumberField(TEXT("x"), FCString::Atof(*XTextBox->GetText().ToString()));
	SelectedWidget->JsonObject->SetNumberField(TEXT("y"), FCString::Atof(*YTextBox->GetText().ToString()));
	SelectedWidget->JsonObject->SetNumberField(TEXT("width"), FCString::Atof(*WidthTextBox->GetText().ToString()));
	SelectedWidget->JsonObject->SetNumberField(TEXT("height"), FCString::Atof(*HeightTextBox->GetText().ToString()));
	SelectedWidget->JsonObject->SetStringField(TEXT("text"), TextTextBox->GetText().ToString());

	// 更新内存中的值
	UpdateWidgetFromJson(SelectedWidget);

	// 写回文件
	if (RootJsonObject.IsValid())
	{
		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), Writer);
		FFileHelper::SaveStringToFile(OutputString, *JsonFilePath);
	}

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(
			FString::Printf(TEXT("已更新: %s"), *SelectedWidget->Name)
		));
	}

	return FReply::Handled();
}

FReply SImageToUMGModifier::OnRegenerateClicked()
{
	RefreshFromJson();

	if (!FPaths::FileExists(JsonFilePath))
	{
		return FReply::Handled();
	}

	FString Message;
	bool bSuccess = FImageToUMGBlueprintGenerator::GenerateFromJson(JsonFilePath, Message);

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(Message));
	}

	return FReply::Handled();
}

FReply SImageToUMGModifier::OnOpenBlueprintClicked()
{
	if (!RootJsonObject.IsValid())
	{
		return FReply::Handled();
	}

	FString OutputPath = TEXT("/Game/UI");
	RootJsonObject->TryGetStringField(TEXT("output_path"), OutputPath);
	FString BlueprintName = TEXT("BP_GeneratedWidget");
	RootJsonObject->TryGetStringField(TEXT("blueprint_name"), BlueprintName);

	FString AssetPath = FString::Printf(TEXT("%s/%s.%s"), *OutputPath, *BlueprintName, *BlueprintName);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));

	if (AssetData.IsValid())
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetData.GetAsset());
	}
	else if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(TEXT("蓝图尚未生成，请先点击重新生成")));
	}

	return FReply::Handled();
}

void SImageToUMGModifier::UpdateWidgetFromJson(TSharedPtr<FWidgetInfo> Info)
{
	if (!Info.IsValid() || !Info->JsonObject.IsValid())
	{
		return;
	}
	Info->JsonObject->TryGetNumberField(TEXT("x"), Info->X);
	Info->JsonObject->TryGetNumberField(TEXT("y"), Info->Y);
	Info->JsonObject->TryGetNumberField(TEXT("width"), Info->Width);
	Info->JsonObject->TryGetNumberField(TEXT("height"), Info->Height);
	Info->JsonObject->TryGetStringField(TEXT("text"), Info->Text);
}
