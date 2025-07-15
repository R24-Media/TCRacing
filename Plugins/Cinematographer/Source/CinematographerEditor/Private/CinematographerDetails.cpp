// Copyright 2022 lumines_labs. All Rights Reserved.

#include "CinematographerDetails.h"

#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/DetailCategoryBuilder.h"
#include "IDetailGroup.h"
#include "IDetailChildrenBuilder.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Editor/PropertyEditor/Public/PropertyHandle.h"

#include "Cinematographer/Public/RealCameraComponent.h"
#include "Cinematographer/Public/RealCameraActor.h"

#define LOCTEXT_NAMESPACE "CinematographerDetails"

static FName NAME_Category(TEXT("Category")); 
static FString ShutterSpeedString(TEXT("ShutterSpeed"));
static FString ShutterAngleString(TEXT("ShutterAngle"));
static FString TargetFPSString(TEXT("TargetFPS"));
static FString SimulatedFPSString(TEXT("SimulatedFPS"));


TSharedRef<IDetailCustomization> FCinematographerDetails::MakeInstance()
{
	return MakeShareable(new FCinematographerDetails);
}

void FCinematographerDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(ObjectsToEdit);

	IDetailCategoryBuilder& CategoryCurrentCameraSettings = DetailBuilder.EditCategory("Current Camera Settings", LOCTEXT("CatName", "Tags"), ECategoryPriority::Default);
	CategoryCurrentCameraSettings.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryTags = DetailBuilder.EditCategory("Tags", LOCTEXT("CatName", "Tags"), ECategoryPriority::Default);
	CategoryTags.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryPhysics = DetailBuilder.EditCategory("Physics", LOCTEXT("CatName", "Physics"), ECategoryPriority::Default);
	CategoryPhysics.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryAssetUserData = DetailBuilder.EditCategory("AssetUserData", LOCTEXT("CatName", "AssetUserData"), ECategoryPriority::Default);
	CategoryAssetUserData.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryActivation = DetailBuilder.EditCategory("Activation", LOCTEXT("CatName", "Activation"), ECategoryPriority::Default);
	CategoryActivation.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryCameraOptions = DetailBuilder.EditCategory("CameraOptions", LOCTEXT("CatName", "CameraOptions"), ECategoryPriority::Default);
	CategoryCameraOptions.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryAutoPlayerActivation = DetailBuilder.EditCategory("AutoPlayerActivation", LOCTEXT("CatName", "AutoPlayerActivation"), ECategoryPriority::Default);
	CategoryAutoPlayerActivation.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryPostProcess = DetailBuilder.EditCategory("PostProcess", LOCTEXT("CatName", "PostProcess"), ECategoryPriority::Default);
	CategoryPostProcess.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryColorGrading = DetailBuilder.EditCategory("Color Grading", LOCTEXT("CatName", "Color Grading"), ECategoryPriority::Default);
	CategoryColorGrading.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryRenderingFeatures = DetailBuilder.EditCategory("Rendering Features", LOCTEXT("CatName", "Legacy Rendering Features"), ECategoryPriority::Default);
	CategoryRenderingFeatures.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryLens = DetailBuilder.EditCategory("Lens", LOCTEXT("CatName", "Lens"), ECategoryPriority::Default);
	CategoryLens.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryCamera = DetailBuilder.EditCategory("Camera", LOCTEXT("CatName", "Camera"), ECategoryPriority::Default);
	CategoryCamera.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryRealCameraActor = DetailBuilder.EditCategory("RealCameraActor", LOCTEXT("CatName", "RealCameraActor"), ECategoryPriority::Default);
	CategoryRealCameraActor.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryGI = DetailBuilder.EditCategory("Global Illumination", LOCTEXT("CatName", "Global Illumination"), ECategoryPriority::Default);
	CategoryGI.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryReflections = DetailBuilder.EditCategory("Reflections", LOCTEXT("CatName", "Reflections"), ECategoryPriority::Default);
	CategoryReflections.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryWorldPartition = DetailBuilder.EditCategory("WorldPartition", LOCTEXT("WorldPartition", "WorldPartition"), ECategoryPriority::Default);
	CategoryWorldPartition.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryHLOD = DetailBuilder.EditCategory("HLOD", LOCTEXT("CatName", "HLOD"), ECategoryPriority::Default);
	CategoryHLOD.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryDataLayers = DetailBuilder.EditCategory("DataLayers", LOCTEXT("CatName", "DataLayers"), ECategoryPriority::Default);
	CategoryDataLayers.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryReplication = DetailBuilder.EditCategory("Replication", LOCTEXT("CatName", "Replication"), ECategoryPriority::Default);
	CategoryReplication.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryActor = DetailBuilder.EditCategory("Actor", LOCTEXT("CatName", "Actor"), ECategoryPriority::Default);
	CategoryActor.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryCooking = DetailBuilder.EditCategory("Cooking", LOCTEXT("CatName", "Cooking"), ECategoryPriority::Default);
	CategoryCooking.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryCollision = DetailBuilder.EditCategory("Collision", LOCTEXT("CatName", "Collision"), ECategoryPriority::Default);
	CategoryCollision.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryLOD = DetailBuilder.EditCategory("LOD", LOCTEXT("CatName", "LOD"), ECategoryPriority::Default);
	CategoryLOD.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategorySC = DetailBuilder.EditCategory("SceneCapture", LOCTEXT("CatName", "SceneCapture"), ECategoryPriority::Default);
	CategorySC.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryProjection = DetailBuilder.EditCategory("Projection", LOCTEXT("CatName", "Projection"), ECategoryPriority::Default);
	CategoryProjection.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryPPV = DetailBuilder.EditCategory("PostProcessVolume", LOCTEXT("CatName", "PostProcessVolume"), ECategoryPriority::Default);
	CategoryPPV.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryPR = DetailBuilder.EditCategory("PlanarReflection", LOCTEXT("CatName", "PlanarReflection"), ECategoryPriority::Default);
	CategoryPR.SetCategoryVisibility(b_LegacySettings);
	IDetailCategoryBuilder& CategoryNW = DetailBuilder.EditCategory("Networking", LOCTEXT("CatName", "Networking"), ECategoryPriority::Default);
	CategoryNW.SetCategoryVisibility(b_LegacySettings);

	IDetailCategoryBuilder & CategoryPostProcessMaterials = DetailBuilder.EditCategory("Post Process Materials", LOCTEXT("CatName", "Post Process Materials"), ECategoryPriority::Default);
	CategoryPostProcessMaterials.SetCategoryVisibility(b_LegacySettings);

	TSharedPtr<IPropertyHandle> RealCameraShutterSettingsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraShutterSettings));
	TSharedPtr<IPropertyHandle> CameraModeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraShutterSettings.CameraMode));
	TSharedPtr<IPropertyHandle> CameraShutterSpeedProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraShutterSettings.CurrentShutterSpeed));
	TSharedPtr<IPropertyHandle> CameraShutterAngleProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraShutterSettings.CurrentShutterAngle));
	TSharedPtr<IPropertyHandle> CameraTargetFPSProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraShutterSettings.TargetFPS));
	TSharedPtr<IPropertyHandle> CameraSimulatedFPSProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraShutterSettings.SimulatedFPS));

	TSharedPtr<IPropertyHandle> CurrentISOProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, CurrentISO));
	TSharedPtr<IPropertyHandle> NDFilterProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, CurrentNDFilter));
	TSharedPtr<IPropertyHandle> CurrentApertureProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, CurrentAperture));
	TSharedPtr<IPropertyHandle> ActiveFocalLengthProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, ActiveFocalLength));
	TSharedPtr<IPropertyHandle> CurrentAutoExposureProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, AutoExposureMode));
	TSharedPtr<IPropertyHandle> CurrentAutoExposureCompensationProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, AutoExposureCompensationSteps));
	

	TSharedPtr<IPropertyHandle> bCustomAspectRatioProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bCustomAspectRatio));
	TSharedPtr<IPropertyHandle> CustomAspectRatioProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, CustomAspectRatio));

	TSharedPtr<IPropertyHandle> CameraBodyProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, ActiveRealCameraBody));
	TSharedPtr<IPropertyHandle> CameraLensProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, ActiveRealCameraLens));

	//Features
	TSharedPtr<IPropertyHandle> ExposureCompensationProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bExposureCompensationEnabled));
	TSharedPtr<IPropertyHandle> DynamicRangeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bSimulatedDynamicRangeEnabled));
	TSharedPtr<IPropertyHandle> ISOGrainEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bISOGrainEnabled));
	TSharedPtr<IPropertyHandle> BarrelDistortionEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bBarrelDistortionEnabled));
	TSharedPtr<IPropertyHandle> BarrelDistortionFocalCompEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bBarrelDistortionFocalCompEnabled));
	TSharedPtr<IPropertyHandle> LensSharpnessEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bLensSharpnessEnabled));
	TSharedPtr<IPropertyHandle> VignettingEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bVignettingEnabled));
	TSharedPtr<IPropertyHandle> PurpleFringingEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bPurpleFringingEnabled));
	TSharedPtr<IPropertyHandle> ChromaticAberationEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bChromaticAberrationEnabled));
	TSharedPtr<IPropertyHandle> MotionBlurEnabledProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, bMotionBlurEnabled));

	IDetailCategoryBuilder& CategoryCameraSetup = DetailBuilder.EditCategory("Camera Setup", LOCTEXT("Camera Setup Name", "Camera Setup"), ECategoryPriority::Important);
	CategoryCameraSetup.AddProperty(CameraBodyProperty);
	CategoryCameraSetup.AddProperty(CameraLensProperty);

	CameraBodyProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::BodyUpdate));
	CameraLensProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::LensUpdate));

	IDetailCategoryBuilder& CategoryFocus = DetailBuilder.EditCategory(TEXT("Focus"), LOCTEXT("Focus", "Focus"), ECategoryPriority::Important);
	CategoryFocus.InitiallyCollapsed(true);

	IDetailCategoryBuilder& CategoryShot = DetailBuilder.EditCategory(TEXT("Configure Shot"), LOCTEXT("Configure Shot", "Configure Shot"), ECategoryPriority::Important);

	TSharedPtr<IPropertyHandle> RealCameraFocusSettingsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(URealCameraComponent, RealCameraFocusSettings));
	CategoryShot.AddProperty(RealCameraFocusSettingsProperty.ToSharedRef());

	CategoryShot.AddProperty(ActiveFocalLengthProperty.ToSharedRef());
	CategoryShot.AddCustomRow(LOCTEXT("", ""), false);
	CategoryShot.AddProperty(CurrentAutoExposureProperty.ToSharedRef());
	CategoryShot.AddProperty(CurrentAutoExposureCompensationProperty.ToSharedRef());
	
	
	CategoryShot.AddCustomRow(LOCTEXT("", ""), false);
	CategoryShot.AddProperty(RealCameraShutterSettingsProperty.ToSharedRef());
	CategoryShot.AddCustomRow(LOCTEXT("", ""), false);
	CategoryShot.AddProperty(CurrentApertureProperty.ToSharedRef());
	CategoryShot.AddProperty(CurrentISOProperty.ToSharedRef());
	
	CurrentAutoExposureProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::AutoExposureUpdate));
	ActiveFocalLengthProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::FocalLengthUpdate));
	CurrentApertureProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ApertureUpdate));
	CurrentISOProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ISOGrainUpdate));
	NDFilterProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ExposureUpdate));

	CameraModeProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ShutterUpdate));
	CameraShutterSpeedProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ShutterUpdate));
	CameraShutterAngleProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ShutterUpdate));
	CameraTargetFPSProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ShutterUpdate));
	CameraSimulatedFPSProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ShutterUpdate));

	IDetailCategoryBuilder& CategoryAspectRatio = DetailBuilder.EditCategory(TEXT("Aspect Ratio"), LOCTEXT("Aspect Ratio", "Aspect Ratio"), ECategoryPriority::Important);
	CategoryAspectRatio.InitiallyCollapsed(true);

	CategoryAspectRatio.AddProperty(bCustomAspectRatioProperty.ToSharedRef());
	CategoryAspectRatio.AddProperty(CustomAspectRatioProperty.ToSharedRef());

	bCustomAspectRatioProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::AspectRatioUpdate));
	CustomAspectRatioProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::AspectRatioUpdate));

	IDetailCategoryBuilder& CategoryCinematographerFeatures = DetailBuilder.EditCategory(TEXT("Cinematographer Features"), LOCTEXT("Cinematographer Features", "Cinematographer Features"), ECategoryPriority::Important);
	CategoryCinematographerFeatures.InitiallyCollapsed(true);
	
	CategoryCinematographerFeatures.AddProperty(ExposureCompensationProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(DynamicRangeProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(ISOGrainEnabledProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(MotionBlurEnabledProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(BarrelDistortionEnabledProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(BarrelDistortionFocalCompEnabledProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(LensSharpnessEnabledProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(VignettingEnabledProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(PurpleFringingEnabledProperty.ToSharedRef());
	CategoryCinematographerFeatures.AddProperty(ChromaticAberationEnabledProperty.ToSharedRef());

	DynamicRangeProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::SimulatedDynamicRangeUpdate));
	ISOGrainEnabledProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ISOGrainUpdate));
	MotionBlurEnabledProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::MotionBlurEnabledUpdate));
	ExposureCompensationProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ExposureUpdate));
	BarrelDistortionEnabledProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::BarrelDistortionUpdate));
	BarrelDistortionFocalCompEnabledProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::BarrelDistortionFocalCompUpdate));
	LensSharpnessEnabledProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::LensSharpnessUpdate));
	VignettingEnabledProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::VignettingUpdate));
	PurpleFringingEnabledProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::PurpleFringingUpdate));
	ChromaticAberationEnabledProperty.ToSharedRef()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCinematographerDetails::ChromaticAberrationUpdate));

	IDetailCategoryBuilder& CategoryLookatTrackingSettings = DetailBuilder.EditCategory(TEXT("LookatTrackingSettings"), LOCTEXT("LookatTrackingSettings", "LookatTrackingSettings"), ECategoryPriority::Important);
	CategoryLookatTrackingSettings.InitiallyCollapsed(true);
	TSharedPtr<IPropertyHandle> RealCameraLookatTrackingSettingsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraLookatTrackingSettings));
	CategoryLookatTrackingSettings.AddProperty(RealCameraLookatTrackingSettingsProperty.ToSharedRef());
}

TSharedRef<IPropertyTypeCustomization> FCameraShutterSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FCameraShutterSettingsCustomization());
}

void FCameraShutterSettingsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,class FDetailWidgetRow& HeaderRow,IPropertyTypeCustomizationUtils& StructCustomizationUtils){}

void FCameraShutterSettingsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// Retrieve structure's child properties
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);
	TMap<FName, TSharedPtr< IPropertyHandle > > PropertyHandles;
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedRef<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		const FName PropertyName = ChildHandle->GetProperty()->GetFName();

		PropertyHandles.Add(PropertyName, ChildHandle);
	}

	// Retrieve special case properties
	CameraModeMethodHandle = PropertyHandles.FindChecked(GET_MEMBER_NAME_CHECKED(FCameraShutterSettings, CameraMode));

	for (auto Iter(PropertyHandles.CreateConstIterator()); Iter; ++Iter)
	{
		// make the widget
		IDetailPropertyRow& PropertyRow = ChildBuilder.AddProperty(Iter.Value().ToSharedRef());

		// set up delegate to know if we need to hide it
		FString const& Category = Iter.Value()->GetMetaData(NAME_Category);
		if (Category == ShutterSpeedString)
		{
			PropertyRow.Visibility(TAttribute<EVisibility>(this, &FCameraShutterSettingsCustomization::IsShutterSpeedGroupVisible));
		}
		else if (Category == ShutterAngleString)
		{
			PropertyRow.Visibility(TAttribute<EVisibility>(this, &FCameraShutterSettingsCustomization::IsShutterAngleGroupVisible));
		}
		else if (Category == TargetFPSString)
		{
			PropertyRow.Visibility(TAttribute<EVisibility>(this, &FCameraShutterSettingsCustomization::IsTargetFPSGroupVisible));
		}
		else if (Category == SimulatedFPSString)
		{
			PropertyRow.Visibility(TAttribute<EVisibility>(this, &FCameraShutterSettingsCustomization::IsSimulatedFPSGroupVisible));
		}
	}
}

EVisibility FCameraShutterSettingsCustomization::IsShutterSpeedGroupVisible() const
{
	uint8 CameraModeMethodNumber;
	CameraModeMethodHandle->GetValue(CameraModeMethodNumber);
	ERealCameraMode const CameraModeMethod = static_cast<ERealCameraMode>(CameraModeMethodNumber);
	if (CameraModeMethod == ERealCameraMode::Photography)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
EVisibility FCameraShutterSettingsCustomization::IsShutterAngleGroupVisible() const
{
	uint8 CameraModeMethodNumber;
	CameraModeMethodHandle->GetValue(CameraModeMethodNumber);
	ERealCameraMode const CameraModeMethod = static_cast<ERealCameraMode>(CameraModeMethodNumber);
	if (CameraModeMethod == ERealCameraMode::Movie || CameraModeMethod == ERealCameraMode::Game)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
EVisibility FCameraShutterSettingsCustomization::IsTargetFPSGroupVisible() const
{
	uint8 CameraModeMethodNumber;
	CameraModeMethodHandle->GetValue(CameraModeMethodNumber);

	ERealCameraMode const CameraModeMethod = static_cast<ERealCameraMode>(CameraModeMethodNumber);
	if (CameraModeMethod == ERealCameraMode::Movie)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
EVisibility FCameraShutterSettingsCustomization::IsSimulatedFPSGroupVisible() const
{
	uint8 CameraModeMethodNumber;
	CameraModeMethodHandle->GetValue(CameraModeMethodNumber);

	ERealCameraMode const CameraModeMethod = static_cast<ERealCameraMode>(CameraModeMethodNumber);
	if (CameraModeMethod == ERealCameraMode::Game)
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

void FCinematographerDetails::LensUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->SetActiveLens();
	}
}

void FCinematographerDetails::BodyUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->SetActiveBody();
	}
}

void FCinematographerDetails::ShutterUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateShutter();
	}
}

void FCinematographerDetails::FocalLengthUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateFocalLength();
	}
}

void FCinematographerDetails::ApertureUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateAperture();
	}
}

void FCinematographerDetails::SimulatedDynamicRangeUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateSimulatedDynamicRange();
	}
}

void FCinematographerDetails::AutoExposureUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateAutoExposure();
	}
}

void FCinematographerDetails::ExposureUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateExposure();
	}
}

void FCinematographerDetails::ISOGrainUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateISO();
	}
}

void FCinematographerDetails::AspectRatioUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateAspectRatio();
	}
}

void FCinematographerDetails::MotionBlurEnabledUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateShutter();
	}
}

void FCinematographerDetails::BarrelDistortionUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateBarrelDistortion();
	}
}

void FCinematographerDetails::BarrelDistortionFocalCompUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateFocalLength();
	}
}

void FCinematographerDetails::LensSharpnessUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateLensSharpness();
	}
}

void FCinematographerDetails::VignettingUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateVignetting();
	}
}

void FCinematographerDetails::PurpleFringingUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdatePurpleFringing();
	}
}

void FCinematographerDetails::ChromaticAberrationUpdate()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->UpdateChromaticAberration();
	}
}

void FCinematographerDetails::LensChange(const FAssetData& AssetData)
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->SetActiveLens();
	}
}

void FCinematographerDetails::BodyChange(const FAssetData& AssetData)
{
	BodyBlueprintAsset = AssetData;
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		RCA->SetActiveBody();
	}
}

EVisibility FCinematographerDetails::bIsMovieMode() const
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
		if (RCA->RealCameraShutterSettings.CameraMode == ERealCameraMode::Photography) return EVisibility::HitTestInvisible;
		else return EVisibility::Visible;
	}
	return EVisibility::Visible;
}

FReply FCinematographerDetails::EditObjects()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{ 
		if (!Object.IsValid()) continue;
		UE_LOG(LogTemp, Warning, TEXT("EditObjects!"));
		ARealCameraActor* RCA = Cast <ARealCameraActor>(Object.Get());
		if (!RCA) continue;
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE