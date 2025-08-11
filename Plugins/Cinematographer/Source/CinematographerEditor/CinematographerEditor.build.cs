// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CinematographerEditor : ModuleRules
{
	public CinematographerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UnrealEd",
			"DetailCustomizations",
			"PropertyEditor",
			"AnimationEditor",
			"EditorStyle",
			"CinematicCamera",
			"Cinematographer",
			"InteractiveToolsFramework",
			"EditorInteractiveToolsFramework",
			"EditorFramework"
		});

		PrivateDependencyModuleNames.AddRange(
			new string[] {
			"Slate",
			"SlateCore",
			"Projects",

		});
	}
}
