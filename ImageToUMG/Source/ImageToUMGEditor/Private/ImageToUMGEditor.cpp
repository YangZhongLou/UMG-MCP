// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImageToUMGEditor.h"
#include "Modules/ModuleManager.h"

class FImageToUMGEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogTemp, Log, TEXT("ImageToUMGEditor module started"));
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_MODULE(FImageToUMGEditorModule, ImageToUMGEditor)
