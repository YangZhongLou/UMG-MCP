// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImageToUMGEditor.h"
#include "ImageToUMGBlueprintGenerator.h"
#include "SImageToUMGModifier.h"
#include "Containers/Ticker.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "WorkspaceMenuStructureModule.h"

static const FName ImageToUMGModifierTabName(TEXT("ImageToUMGModifier"));

class FImageToUMGEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogTemp, Log, TEXT("ImageToUMGEditor module started"));

		// 注册编辑器标签页
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
			ImageToUMGModifierTabName,
			FOnSpawnTab::CreateRaw(this, &FImageToUMGEditorModule::SpawnModifierTab)
		)
		.SetDisplayName(FText::FromString(TEXT("ImageToUMG 微调")))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

		// 注册 Window 菜单项
		UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = WindowMenu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntry(
			"OpenImageToUMGModifier",
			FText::FromString(TEXT("ImageToUMG 微调面板")),
			FText::FromString(TEXT("打开 ImageToUMG 蓝图微调面板")),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FImageToUMGEditorModule::OpenModifierTab))
		);

		// 注册定时器，每 2 秒检查一次 trigger 文件
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateRaw(this, &FImageToUMGEditorModule::Tick),
			2.0f
		);
	}

	virtual void ShutdownModule() override
	{
		if (TickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
			TickerHandle.Reset();
		}

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ImageToUMGModifierTabName);
	}

private:
	FTSTicker::FDelegateHandle TickerHandle;
	TSharedPtr<SImageToUMGModifier> ModifierWidget;

	bool Tick(float DeltaTime)
	{
		CheckTriggerFile();
		return true;
	}

	void CheckTriggerFile()
	{
		const FString TriggerPath = FPaths::ProjectContentDir() / TEXT("ImageToUMG_Trigger.tmp");
		const FString JsonPath = FPaths::ProjectContentDir() / TEXT("ImageToUMG_WidgetGen.json");

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.FileExists(*TriggerPath))
		{
			return;
		}

		if (!PlatformFile.FileExists(*JsonPath))
		{
			PlatformFile.DeleteFile(*TriggerPath);
			UE_LOG(LogTemp, Warning, TEXT("Trigger 文件存在但 JSON 文件不存在"));
			return;
		}

		// 执行生成
		FString Message;
		bool bSuccess = FImageToUMGBlueprintGenerator::GenerateFromJson(JsonPath, Message);

		// 删除 trigger 文件
		PlatformFile.DeleteFile(*TriggerPath);

		// 刷新微调面板
		if (ModifierWidget.IsValid())
		{
			ModifierWidget->RefreshFromJson();
		}

		// 显示通知
		FNotificationInfo Info(FText::FromString(Message));
		Info.ExpireDuration = bSuccess ? 5.0f : 8.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		UE_LOG(LogTemp, Log, TEXT("ImageToUMG: %s"), *Message);
	}

	TSharedRef<SDockTab> SpawnModifierTab(const FSpawnTabArgs& SpawnTabArgs)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SAssignNew(ModifierWidget, SImageToUMGModifier)
			];
	}

	void OpenModifierTab()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(ImageToUMGModifierTabName);
	}
};

IMPLEMENT_MODULE(FImageToUMGEditorModule, ImageToUMGEditor)
