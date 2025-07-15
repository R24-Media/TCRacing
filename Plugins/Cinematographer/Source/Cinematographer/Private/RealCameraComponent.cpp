// Copyright 2022 lumines_labs. All Rights Reserved.

#include "RealCameraComponent.h"

//#include “Runtime/Launch/Resources/Version.h”

URealCameraComponent::URealCameraComponent()
{
    PostProcessSettings.bOverride_AutoExposureMethod = true;
    PostProcessSettings.AutoExposureMethod = AEM_Manual;
    PostProcessSettings.bOverride_CameraISO = true;
    PostProcessSettings.bOverride_AutoExposureBias = true;
    PostProcessSettings.bOverride_CameraShutterSpeed = true;
    PostProcessSettings.bOverride_DepthOfFieldFstop = true;
    PostProcessSettings.bOverride_SceneFringeIntensity = true;
    PostProcessSettings.bOverride_ChromaticAberrationStartOffset = true;
    PostProcessSettings.ChromaticAberrationStartOffset = 0.5f;
    PostProcessSettings.bOverride_VignetteIntensity = true;
    PostProcessSettings.bOverride_MotionBlurAmount = true;
    PostProcessSettings.bOverride_MotionBlurTargetFPS = true;
    PostProcessSettings.bOverride_MotionBlurMax = true;
    PostProcessSettings.MotionBlurMax = 10.0f;
    PostProcessSettings.bOverride_FilmSlope = true;
    PostProcessSettings.bOverride_FilmToe = true;
    LensSettings.MinFocalLength = 0.00000001;
    LensSettings.MaxFocalLength = 1000000000;
}

void URealCameraComponent::UpdateFocus()
{
    
    FocusSettings = RealCameraFocusSettings;
    RealCameraActor = Cast<ARealCameraActor>(GetOwner());
    if (RealCameraActor && RealCameraActor->ActiveLens)
    {
        if (RealCameraActor->ActiveLens && FocusSettings.ManualFocusDistance < RealCameraActor->ActiveLens->FocusDistanceMin)
        {
            FocusSettings.ManualFocusDistance = RealCameraActor->ActiveLens->FocusDistanceMin;
            RealCameraFocusSettings.ManualFocusDistance = RealCameraActor->ActiveLens->FocusDistanceMin;
        } 
    }
}

#if WITH_EDITOR
void URealCameraComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyChangedName = PropertyChangedEvent.GetPropertyName();
	if (PropertyChangedName == "FocusMethod" || PropertyChangedName == "ManualFocusDistance" || PropertyChangedName == "TrackingFocusSettings" 
        || PropertyChangedName == "bDrawDebugFocusPlane" || PropertyChangedName == "DebugFocusPlaneColor" 
        || PropertyChangedName == "bSmoothFocusChanges" || PropertyChangedName == "FocusSmoothingInterpSpeed" || PropertyChangedName == "FocusOffset")
	{
		UpdateFocus();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
