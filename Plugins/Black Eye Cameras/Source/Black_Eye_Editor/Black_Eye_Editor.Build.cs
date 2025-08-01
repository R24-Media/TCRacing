// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

using UnrealBuildTool;

public class Black_Eye_Editor : ModuleRules
{
    public Black_Eye_Editor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;


        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            }
            );
                
        
        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
            );
            
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "InputCore",
                "Projects",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "ComponentVisualizers",
                "Black_Eye",
                "EditorSubsystem",
                "ToolMenus",
                //"LevelEditor",
                "PropertyEditor"
                // ... add other public dependencies that you statically link with here ...
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {

                // ... add private dependencies that you statically link with here ...	
            }
            );
        
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );

        //bUsePrecompiled = true;
        bUseUnity = false;
    }
}
