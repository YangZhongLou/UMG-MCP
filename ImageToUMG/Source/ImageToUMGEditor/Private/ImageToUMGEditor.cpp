// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImageToUMGEditor.h"
#include "ImageToUMGBlueprintGenerator.h"
#include "Containers/Ticker.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

class FImageToUMGEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogTemp, Log, TEXT("ImageToUMGEditor module started"));

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
	}

private:
	FTSTicker::FDelegateHandle TickerHandle;

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

		// 显示通知
		FNotificationInfo Info(FText::FromString(Message));
		Info.ExpireDuration = bSuccess ? 5.0f : 8.0f;
		Info.bUseSuccessFailIcons = true;
		FSlateNotificationManager::Get().AddNotification(Info);

		UE_LOG(LogTemp, Log, TEXT("ImageToUMG: %s"), *Message);
	}
};

IMPLEMENT_MODULE(FImageToUMGEditorModule, ImageToUMGEditor)
