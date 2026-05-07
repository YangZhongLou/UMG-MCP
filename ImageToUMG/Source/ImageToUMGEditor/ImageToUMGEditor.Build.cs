// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ImageToUMGEditor : ModuleRules
{
	public ImageToUMGEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Slate",
			"SlateCore",
			"UMG",
			"UnrealEd",
			"ToolMenus",
			"EditorSubsystem",
			"Projects",
			"BlueprintGraph",
			"Kismet",
			"KismetCompiler",
			"ImageToUMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ApplicationCore",
			"WorkspaceMenuStructure",
			"LevelEditor",
			"InteractiveToolsFramework",
			"EditorInteractiveToolsFramework"
		});
	}
}
