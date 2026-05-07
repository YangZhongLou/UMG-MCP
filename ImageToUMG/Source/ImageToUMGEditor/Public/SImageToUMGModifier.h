// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SEditableTextBox.h"

struct FWidgetInfo
{
	FString Name;
	FString Type;
	float X = 0.0f;
	float Y = 0.0f;
	float Width = 100.0f;
	float Height = 40.0f;
	FString Text;
	TSharedPtr<FJsonObject> JsonObject;
};

class SImageToUMGModifier : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SImageToUMGModifier) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void RefreshFromJson();

private:
	TArray<TSharedPtr<FWidgetInfo>> WidgetList;
	TSharedPtr<SListView<TSharedPtr<FWidgetInfo>>> ListView;
	TSharedPtr<FJsonObject> RootJsonObject;
	FString JsonFilePath;
	TSharedPtr<FWidgetInfo> SelectedWidget;

	TSharedPtr<SEditableTextBox> XTextBox;
	TSharedPtr<SEditableTextBox> YTextBox;
	TSharedPtr<SEditableTextBox> WidthTextBox;
	TSharedPtr<SEditableTextBox> HeightTextBox;
	TSharedPtr<SEditableTextBox> TextTextBox;
	TSharedPtr<STextBlock> StatusText;

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FWidgetInfo> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnWidgetSelected(TSharedPtr<FWidgetInfo> Item, ESelectInfo::Type SelectInfo);

	FReply OnApplyClicked();
	FReply OnRegenerateClicked();
	FReply OnOpenBlueprintClicked();
	void ParseWidgetsRecursive(const TSharedPtr<FJsonObject>& JsonObj);
	void UpdateWidgetFromJson(TSharedPtr<FWidgetInfo> Info);
};
