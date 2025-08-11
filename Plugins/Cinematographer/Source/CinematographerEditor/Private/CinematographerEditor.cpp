// Copyright 2022 lumines_labs. All Rights Reserved.

#include "CinematographerEditor.h"
#include "Cinematographer/Public/RealCameraActor.h"
#include "Cinematographer/Public/RealCameraComponent.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Editor/PropertyEditor/Public/PropertyEditorDelegates.h"
#include "CinematographerDetails.h"

//Icon
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "FReaLCameraEditorModule"

void FCinematographerEditorModule::StartupModule()
{
	StyleSet = MakeShareable(new FSlateStyleSet("RealCameraStyle"));
	
	FString ContentDir = IPluginManager::Get().FindPlugin("Cinematographer")->GetBaseDir();
	StyleSet->SetContentRoot(ContentDir);

	FSlateImageBrush* RealCameraActorThumbnailBrush = new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Resources/RealCameraActor_Icon128"), TEXT(".png")), FVector2D(128.f, 128.f));
	FSlateImageBrush* RealCameraBodyThumbnailBrush = new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Resources/RealCameraBody_Icon128"), TEXT(".png")), FVector2D(128.f, 128.f));
	FSlateImageBrush* RealCameraLensThumbnailBrush = new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Resources/RealCameraLens_Icon128"), TEXT(".png")), FVector2D(128.f, 128.f));

	if (RealCameraActorThumbnailBrush)
	{
		StyleSet->Set("ClassThumbnail.RealCameraActor", RealCameraActorThumbnailBrush);
	}
	if (RealCameraBodyThumbnailBrush)
	{
		StyleSet->Set("ClassThumbnail.RealCameraBody", RealCameraBodyThumbnailBrush);
	}
	if (RealCameraLensThumbnailBrush)
	{
		StyleSet->Set("ClassThumbnail.RealCameraLens", RealCameraLensThumbnailBrush);
	}
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(ARealCameraActor::StaticClass()->GetFName(),FOnGetDetailCustomizationInstance::CreateStatic(&FCinematographerDetails::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout(FCameraShutterSettings::StaticStruct()->GetFName(),FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCameraShutterSettingsCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FCinematographerEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("ARealCameraActor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("FCameraShutterSettings");
		PropertyModule.NotifyCustomizationModuleChanged();
	}
}
IMPLEMENT_MODULE(FCinematographerEditorModule, CinematographerEditor);