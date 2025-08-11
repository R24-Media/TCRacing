// Copyright 2022 lumines_labs. All Rights Reserved.

#pragma once

#include "Editor/PropertyEditor/Public/IDetailCustomization.h"

class FReply;
class URealCameraLens;
class URealCameraBody;

DECLARE_DELEGATE(RefreshOne);

class FCinematographerDetails : public IDetailCustomization
{
public :
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	FReply EditObjects();
	
private:
	RefreshOne one;
	
	TSubclassOf<URealCameraLens>  URealCameraLensClass;
	TSubclassOf<URealCameraBody>  URealCameraBodyClass;
	EVisibility bIsMovieMode() const;

	TArray<TWeakObjectPtr<UObject>> ObjectsToEdit;
	
	UBlueprint* BPBody;

	bool b_LegacySettings = false;

	void LensUpdate();
	void BodyUpdate();
	void ApertureUpdate();
	void ShutterUpdate();
	void FocalLengthUpdate();
	void AutoExposureUpdate();
	void ExposureUpdate();
	void SimulatedDynamicRangeUpdate();
	void ISOGrainUpdate();
	void AspectRatioUpdate();
	void MotionBlurEnabledUpdate();
	void BarrelDistortionUpdate();
	void BarrelDistortionFocalCompUpdate();
	void LensSharpnessUpdate();
	void VignettingUpdate();
	void PurpleFringingUpdate();
	void ChromaticAberrationUpdate();

	void LensChange(const FAssetData& AssetData);
	void BodyChange(const FAssetData& AssetData);

	bool bIsPrimeLens;

protected:
	FAssetData BodyBlueprintAsset;
}
;

class FCameraShutterSettingsCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,class FDetailWidgetRow& HeaderRow,IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

protected:
	TSharedPtr<IPropertyHandle> CameraModeMethodHandle;

private:
	EVisibility IsShutterSpeedGroupVisible() const;
	EVisibility IsShutterAngleGroupVisible() const;
	EVisibility IsTargetFPSGroupVisible() const;
	EVisibility IsSimulatedFPSGroupVisible() const;
};