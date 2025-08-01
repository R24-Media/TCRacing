// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "Black_Eye_Editor.h"

#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"

#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#include "Editor/UnrealEd/Public/UnrealEdGlobals.h"

#include "ClassIconFinder.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"

#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

#include "LevelEditor.h"

#include "Components/LookAtComponent.h"

#include "Utility/BlackEyeSimpleTarget.h"
#include "Utility/BlackEyeTarget.h"
#include "Utility/BlackEyeWeightedTarget.h"

#include "BlackEyeTargetRowCustomization.h"
#include "BlackEyePersistChangesSubsystem.h"
#include "BlackEyeUICommands.h"

#include "BlackEyeLookAtCustomization.h"

IMPLEMENT_MODULE(FBlack_Eye_EditorModule, Black_Eye_Editor)

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

#define LOCTEXT_NAMESPACE "Black_Eye_EditorModule"

static TSharedPtr<FSlateStyleSet> StyleSetInstance = nullptr;
#define SLATE_IMAGE_BRUSH( ImagePath, ImageSize ) new FSlateImageBrush( StyleSetInstance->RootToContentDir( TEXT(ImagePath), TEXT(".png")), ImageSize)

void FBlack_Eye_EditorModule::StartupModule()
{
    // Create the new style set for our component icons
    StyleSetInstance = MakeShareable(new FSlateStyleSet("BlackEyeEditorStyle"));

    // Assign the content root of this style set
    FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Black_Eye"))->GetContentDir();
    //Icon40x40
    StyleSetInstance->SetContentRoot(ContentDir / TEXT("Editor/Slate"));

    FSlateImageBrush* Icon = SLATE_IMAGE_BRUSH("/Icons/CameraRig", Icon16x16);
    FSlateImageBrush* SaveInPlayIcon = SLATE_IMAGE_BRUSH("/Icons/SIP", Icon40x40);

    // Modify the class icons to use our new awesome icons
    StyleSetInstance->Set("ClassIcon.BlackEyeCameraRigBase", Icon);
    StyleSetInstance->Set("BET.SIP", SaveInPlayIcon);

    // Finally register the style set so it is actually used
    FSlateStyleRegistry::RegisterSlateStyle(*StyleSetInstance);

    // Make a new instance of the visualizers and register them
    if (GUnrealEd)
    {
        FBlackEyeUICommands::Register();

        // Register a function to be called when menu system is initialized
        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(
                                                this, &FBlack_Eye_EditorModule::RegisterMenuExtensions));

        // Register custom property layout instances
        FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
        if (PropertyModule)
        {
            FOnGetPropertyTypeCustomizationInstance TargetPropertyCustomizationInstance = 
                                    FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FBlackEyeTargetRowCustomization::MakeInstance);

            PropertyModule->RegisterCustomPropertyTypeLayout(FBlackEyeSimpleTarget::StaticStruct()->GetFName(), TargetPropertyCustomizationInstance);
            PropertyModule->RegisterCustomPropertyTypeLayout(FBlackEyeWeightedTarget::StaticStruct()->GetFName(), TargetPropertyCustomizationInstance);
            PropertyModule->RegisterCustomPropertyTypeLayout(FBlackEyeTarget::StaticStruct()->GetFName(), TargetPropertyCustomizationInstance);

            PropertyModule->RegisterCustomClassLayout(ULookAtComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FBlackEyeLookAtCustomization::MakeInstance));

            PropertyModule->NotifyCustomizationModuleChanged();
        }
    }
}

void FBlack_Eye_EditorModule::ShutdownModule()
{
    FModuleManager::Get().OnModulesChanged().RemoveAll(this);

    // Unregister the style set and reset the pointer
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSetInstance.Get());
    StyleSetInstance.Reset();

    if (GUnrealEd)
    {
        // Unregister all our menu extensions
        UToolMenus::UnregisterOwner(this);

        // Unregister custom property type layouts
        FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
        if (PropertyModule)
        {
            PropertyModule->UnregisterCustomPropertyTypeLayout(FBlackEyeSimpleTarget::StaticStruct()->GetFName());
            PropertyModule->UnregisterCustomPropertyTypeLayout(FBlackEyeWeightedTarget::StaticStruct()->GetFName());
            PropertyModule->UnregisterCustomPropertyTypeLayout(FBlackEyeTarget::StaticStruct()->GetFName());

            PropertyModule->UnregisterCustomClassLayout(ULookAtComponent::StaticClass()->GetFName());

            PropertyModule->NotifyCustomizationModuleChanged();
        }
    }
}

void FBlack_Eye_EditorModule::RegisterMenuExtensions()
{
    if (FModuleManager::Get().ModuleExists(TEXT("LevelEditor")))
    {
        // Use the current object as the owner of the menus
        // This allows us to remove all our custom menus when the 
        // module is unloaded
        FToolMenuOwnerScoped OwnerScoped(this);

        // Extend the "File" section of the main toolbar
        UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.AssetsToolbar");
        FToolMenuSection& ToolbarSection = ToolbarMenu->FindOrAddSection("Save In Play");

        const FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
        const TSharedRef<FUICommandList> Commands = LevelEditor->GetGlobalLevelEditorActions();

        Commands->MapAction(
            FBlackEyeUICommands::Get().SaveInPlayToggleButton,
            FExecuteAction::CreateLambda([this]() {
                UBlackEyePersistChangesSubsystem* SaveInPlaySubsystem = GEditor->GetEditorSubsystem<UBlackEyePersistChangesSubsystem>();
                SaveInPlaySubsystem->SetSaveInPlayState(!SaveInPlaySubsystem->IsSaveInPlayEnabled());
                }),
            FCanExecuteAction(),
            FIsActionChecked::CreateLambda([this]() {
                return GEditor->GetEditorSubsystem<UBlackEyePersistChangesSubsystem>()->IsSaveInPlayEnabled();
                })
        );

        ToolbarSection.AddEntry(FToolMenuEntry::InitToolBarButton(
            FBlackEyeUICommands::Get().SaveInPlayToggleButton,
            TAttribute<FText>(), // Label override
            TAttribute<FText>(), // Tooltip override
            FSlateIcon("BlackEyeEditorStyle", "BET.SIP")
        ));
    }
}

#undef LOCTEXT_NAMESPACE
