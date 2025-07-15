// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;



public class Cinematographer : ModuleRules
{
	public Cinematographer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		//PrivateIncludePaths.Add();

		PublicIncludePaths.AddRange(
			new string[] {
				"Runtime/Renderer/Private",
			}
			);

		
		PrivateIncludePaths.AddRange(
			new string[] {		 
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Renderer",
			}
			);

		if (Target.Type == TargetType.Editor) // Is UBT building for Editor ?
		{
			PublicDependencyModuleNames.Add("UnrealEd");
		}

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"CinematicCamera",
				"AnimationEditor"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]{
			}
			);
	}
}

