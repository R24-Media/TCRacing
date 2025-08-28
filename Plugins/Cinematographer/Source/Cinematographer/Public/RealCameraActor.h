// Copyright 2025 lumines_labs. All Rights Reserved.

#pragma once

#include "CineCameraComponent.h"
#include "CineCameraActor.h"

//AutoExposure
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/Material.h"
#include "Engine/TextureRenderTarget2D.h"

#include "LensFile.h"
#include "LensComponent.h"

#include "RealCameraBody.h"
#include "RealCameraLens.h"
#include "RealCameraActor.generated.h"

class URealCameraComponent;
struct FVector2DHalf;

UENUM()
enum class ERealCameraMode : uint8
{
	Photography,
	Movie,
	Game,
	MAX UMETA(Hidden)
};


USTRUCT(BlueprintType)
struct FCameraShutterSettings
{
	GENERATED_USTRUCT_BODY()

	//Photography for still images, Movie for film and Game for interactive media.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography", meta = (DisplayName = "Shutter Mode"))
	ERealCameraMode CameraMode = ERealCameraMode::Photography;

	//Fractions of a second in which light can hit the sensor. So 60 is 1/60 of a second. Low shutter speed -> higher exposure and more motion blur. High shutter speed -> lower exposure and less motion blur.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShutterSpeed", meta = (DisplayName = "Shutter Speed"))
	int32 CurrentShutterSpeed = 125;

	//Motion blur will emulate the look appropriate to the Target FPS independent of the actual frame rate. Give your output a filmic and cinematic look and match the look with already existing content. This setting does not change or overwrite your export frame rate!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetFPS",meta=(EditCondition="bTragetFPSVisible"))
	float TargetFPS = 24;

	UPROPERTY()
	bool bTragetFPSVisible = true;

	//Motion blur will emulate the look appropriate to the Simulated FPS independent of the actual frame rate. This allows for a filmic and cinematic look while maintaining high frame rate in interactive media. This setting does not modify your frame rate!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SimulatedFPS")
	float SimulatedFPS = 60;

	//Describes for how long of the shutter speed the sensor will receive light in degrees. 360¡ -> the sensor gets lit for the full duration the shutter is open. 180¡ ->  the sensor gets lit for half the duration the shutter is open. 45¡-> the sensor gets lit for an eighth of the duration the shutter is open. This informs exposure as well as the amount of motion blur. The most common shutter angle is 180¡ and the resulting image and motion blur will instantly feel familiar, cinematic and of high production value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShutterAngle", meta = (DisplayName = "Shutter Angle"))
	int32 CurrentShutterAngle = 180;
};

UENUM()
namespace EAutoExposureMode
{
	enum Type
	{
		//Shutter speed, aperture and ISO will be controlled by the camera.
		FullAutomatic UMETA(DisplayName = "Full Automatic"),
		//Shutter speed and ISO will be controlled by the camera while aperture can be set by the user.
		AperturePriority UMETA(DisplayName = "Aperture Priority"),
		//Aperture and ISO will be controlled by the camera while shutter speed can be set by the user.
		ShutterPriority UMETA(DisplayName = "Shutter Priority"),
		Off
	};
}

UENUM()
enum class ECompensationSteps : uint8
{
	Three   UMETA(DisplayName = "3"),
	TwoAndTwoThirds  UMETA(DisplayName = "2.66"),
	TwoAndOneThird  UMETA(DisplayName = "2.33"),
	Two  UMETA(DisplayName = "2"),
	OneAndTwoThirds  UMETA(DisplayName = "1.66"),
	OneAndEonThird  UMETA(DisplayName = "1.33"),
	One  UMETA(DisplayName = "1"),
	TwoThirds  UMETA(DisplayName = "0.66"),
	OneThird  UMETA(DisplayName = "0.33"),
	Zero  UMETA(DisplayName = "0"),
	MinusOneThird  UMETA(DisplayName = "-0.33"),
	MinusTwoThirds  UMETA(DisplayName = "-0.66"),
	MinusOne  UMETA(DisplayName = "-1"),
	MinusOneAndEonThird  UMETA(DisplayName = "-1.33"),
	MinusOneAndTwoThirds  UMETA(DisplayName = "-1.66"),
	MinusTwo  UMETA(DisplayName = "-2"),
	MinusTwoAndOneThird  UMETA(DisplayName = "-2.33"),
	MinusTwoAndTwoThirds  UMETA(DisplayName = "-2.66"),
	MinusThree   UMETA(DisplayName = "-3"),
};

UCLASS(Blueprintable)
class CINEMATOGRAPHER_API  ARealCameraActor : public ACineCameraActor
{
	GENERATED_BODY()

public:
	ARealCameraActor(const FObjectInitializer& ObjectInitializer);
	void BeginPlay() override;

	void RealCameraInitilize();
	void RealCaptureInitilize();

	void AutoExposureInit();
	void AutoExposureUpdate();
	void StopUp(int& ISOPosition, int& ShutterPosition, int& AperturePosition);
	void StopDown(int& ISOPosition, int& ShutterPosition, int& AperturePosition);
	void ISODecrease();
	void ISOIncrease();
	void ShutterDecrease();
	void ShutterIncrease();
	void ApertureDecrease();
	void ApertureIncrease();

	void UpdateAutoExposure();
	void SetActiveBody();
	void RealBodyUpdate();
	void SetActiveLens();
	void RealLensUpdate();
	
	void UpdateShutter();
	void UpdateAperture();
	void UpdateAperture(float NewAperture);
	void UpdateApertureRange();
	void UpdateFocalLength();
	void UpdateISO();
	void UpdateExposure();
	void UpdateAutoExposureCompensation();
	void UpdateSimulatedDynamicRange();
	void UpdateAspectRatio();
	void UpdateAspectRatioRenderTexture();
	void UpdateBarrelDistortion();
	void UpdateLensSharpness();
	void UpdateVignetting();
	void UpdatePurpleFringing();
	void UpdateChromaticAberration();
	void UpdateDoFBlades();
	void UpdateISORange();
	void UpdateSensorSize();
	void UpdateLensFile();

	void UpdateFilm();
	void UpdateLUT();
	

	UPROPERTY()
	TArray<float> FStops = { 0.7f, 0.8f, 0.9f, 1.f, 1.1f, 1.2f, 1.4f, 1.6f, 1.8f, 2.f, 2.2f, 2.5f, 2.8f, 3.2f, 3.5f, 4.f, 4.5f, 5.f, 5.6f, 6.3f, 7.1f, 8.f, 9.f, 10.f, 11.f, 13.f, 14.f, 16.f, 18.f, 20.f, 22.f, 25.f, 29.f, 32.f, 36.f, 40.f, 45.f, 51.f, 64.f};

	UPROPERTY()
	TArray<int32 > Shutterspeeds = { 30, 40, 50, 60, 80, 100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000};

	UPROPERTY()
	TArray<int32 > ISOs = {405160, 341200, 273000, 204800, 170600, 136400, 102400, 85200, 68200, 51200, 42600, 34200, 25600, 20000, 16000, 12800, 10000, 8000, 6400, 5000, 4000, 3200, 2500, 2000, 1600, 1250, 1000, 800, 640, 500, 400, 320, 250, 200, 160, 125, 100, 80, 60, 50, 40, 30, 25, 20, 15, 13, 10, 8, 6, 5, 4};

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Item Config")
	class USceneCaptureComponent2D* RealCameraCaptureComponent;

	//Choose one of the predefined Camera Bodies from the "Cinematographer Content/CameraBodies" folderor your own creations.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Camera Setup", meta = (DisplayThumbnail = "true", DisplayName = "RealCamera Body"))
	TSubclassOf<URealCameraBody> ActiveRealCameraBody = nullptr;

	//Choose one of the predefined Lenses from the "Cinematographer Content/Lenses" folder or your own creations. Greyed out, if the camera body has a built-in lens.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Camera Setup", meta = (DisplayThumbnail = "true", EditCondition = "bCurrentLensIsInterchangable", EditConditionHides, DisplayName = "RealCamera Lens"))
	TSubclassOf<URealCameraLens> ActiveRealCameraLens = nullptr;

	//Unavailable if the RealCamera Body has a built-in lens.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Camera Setup", meta = (DisplayThumbnail = "true", EditCondition = "!bCurrentLensIsInterchangable", EditConditionHides, DisplayName = "RealCamera Lens"))
	FName ActiveRealCameraLensName = TEXT("Photography Cropped Sensor Body");

	UPROPERTY()
	class URealCameraBody* ActiveBody;

	UPROPERTY()
	class URealCameraLens* ActiveLens;

	UPROPERTY()
	int EV100ISOPoisiton = 35;
	UPROPERTY()
	int EV100ShutterPoisiton = -15;
	UPROPERTY()
	int EV100AperturePoisiton = 3;

	UPROPERTY()
	bool bRealCameraInitialized = false;

	UPROPERTY()
	float AutoExposureLuminanceTarget = 0.25;

	UPROPERTY()
	float AutoExposureLuminanceTargetLegacy = 0.25;

	UPROPERTY()
	float AutoExposureLuminanceTargetLumen = 0.125;

	UPROPERTY()
	float AutoExposureLuminanceThreshold = .045f;

	UPROPERTY()
	int32 EVRange = 0;

	UPROPERTY()
	int32 EVISORange = 0;

	UPROPERTY()
	FIntPoint EVISORangeMinMax = { 0, 0 };

	UPROPERTY()
	int32 EVShutterRange = 0;

	UPROPERTY()
	FIntPoint EVShutterRangeMinMax = { 0, 0 };

	UPROPERTY()
	int32 EVApertureRange = 0;
	UPROPERTY()
	FIntPoint EVApertureRangeMinMax = { 0, 0 };

	UPROPERTY()
	int32 EVApertureBasePosition = 0;
	UPROPERTY()
	int32 EVApertureCompensationPosition = 0;

	UPROPERTY()
	int32 EVShutterBasePosition = 0;
	UPROPERTY()
	int32 EVShutterCompensationPosition = 0;

	UPROPERTY()
	int32 EVISOCompensationPosition = 0;
	UPROPERTY()
	int32 EVISOBasePosition = 0;

	UPROPERTY()
	float CurrentEV;
	UPROPERTY()
	float BaseEV;
	UPROPERTY()
	float CompensationEV;

	UPROPERTY()
	bool bIsCompensationUpdate;

	UPROPERTY()
	bool bAutoExposureUpdatedThisTick = false;

	//Brighten or darken the results of the auto exposure by this EV.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configure Shot", meta = (DisplayName = "Auto Exposure Compensation"))
	ECompensationSteps AutoExposureCompensationSteps = ECompensationSteps::Zero;

	//Photography for still images, Movie for film and Game for interactive media.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configure Shot")
	FCameraShutterSettings RealCameraShutterSettings;

	//The camera will take full or partial control of shuter, apertue and ISO to continuously expose your shots right.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configure Shot")
	TEnumAsByte<EAutoExposureMode::Type> AutoExposureMode = EAutoExposureMode::FullAutomatic;

	UPROPERTY()
	bool bIsPrimeLens = false;

	//Focal length of your current lens. Greyed out, if the lens is a prime lens and has only one focal length.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Configure Shot", meta = (DisplayName = "Focal Length", ClampMin = "0.001", ForceUnits = mm))
	float ActiveFocalLength = 48.0f;

	UFUNCTION(CallInEditor)
	void SetActiveFocalLength(float value);


	//Sets the aperture of the shot. This setting influences the exposure as well as the depth of field. Greyed out, if the the lens has only one aperture or Auto Exposure is active.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Configure Shot", meta = (DisplayName = "Aperture"))
	float CurrentAperture = 2.8f;

	UFUNCTION(CallInEditor)
	void SetCurrentAperture(float value);

	//Sets the ISO of the shot. This Settings influences the exposure as well as the amount of digital film grain. A higher value amplifies the sensors readings which results in a brighter image but produces more false measurements, resulting in a grainy image. The lowest possible ISO is in most cases recommended.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Configure Shot", meta = (DisplayName = "ISO"))
	int32 CurrentISO = 100;

	UFUNCTION(CallInEditor)
	void SetCurrentISO(int32 value);

	//Neutral-density filter are put in front of the lens to reduce the amount auf light hitting the sensor. ND Filters Values are in EV and the final image exposure will be reduced by this EV.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Configure Shot", meta = (DisplayName = "ND Filter"))
	float CurrentNDFilter = 0;

	UFUNCTION(CallInEditor)
	void SetCurrentNDFilter(float value);

	//Allows cropping the image to a different aspect ratio than the sensors original one.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configure Shot", meta = (DisplayName = "Custom Aspect Ratio enabled", ScriptName = "boolCustomAspectRatio"))
	bool bCustomAspectRatio = false;

	//For 16:9 -> X=16 Y=9.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Configure Shot", meta = (EditCondition = "bCustomAspectRatio"))
	FVector2D CustomAspectRatio { 16.0f, 9.0f};

	UFUNCTION(CallInEditor)
	void SetCustomAspectRatio(FVector2D value);

	UPROPERTY()
	float ShutterExposureCompensation = .0f;

	UPROPERTY()
	FIntPoint CurrentISORange = { 100, 64000 };

	UPROPERTY()
	FVector2D CurrentApertureRange = { 1.8f, 51200};

	UPROPERTY(VisibleAnywhere, Category = "Current Camera Settings")
	bool bCurrentLensIsInterchangable = true;

	UPROPERTY()
	float FocalLenghtCompensation = 0.0f;

	UPROPERTY()
	float ISO_Max = 0.5;

	UPROPERTY()
	float CurrentISOMultiplier = 0.5;

	UPROPERTY()
	float CurrentPixelSize = 0.0f;

	UPROPERTY()
	float CurrentCropFactor = 1.0f;

	UPROPERTY()
	float CurrentBarrelDistortion = 0.15f;

	UPROPERTY()
	float CurrentGeneralLensSharpness = 0.3f;

	UPROPERTY()
	float CurrentCornerLensSharpness = 0.3f;

	UPROPERTY()
	float CurrentPurpleFringing = 0.2f;

	UPROPERTY()
	float CurrentVignetting = 0.35f;

	UPROPERTY()
	float CurrentChromaticAberration = 0.2f;

	UPROPERTY()
	int8 CurrentBladeCount = 7;

	UPROPERTY()
	FVector2D FullFrameSizeSize = { 36.0f, 24.0f };

	UPROPERTY()
	bool UseEditorTick = true;

	UPROPERTY()
	bool bCurrentLensFileInUse = false;

	//RealCamera Function Toggles

	//Enables the contribution of Shutter Angle and ND Filter to the exposure of the shot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Exposure Correction enabled"))
	bool bExposureCompensationEnabled = true;

	//Enables the unique contrast profile of a cameras dynamic range.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Simulated Dynamic Range enabled"))
	bool bSimulatedDynamicRangeEnabled = true;

	//Enables Motion Blur.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Motion Blur enabled"))
	bool bMotionBlurEnabled = true;

	//Enables Barrel/Pincushion Distortion. Greyed out if the used RealCamera Lens uses a LensFile. Post process distorion is then disabled and the more precise distortion of the LensFile is used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Barrel/Pincushion Distortion enabled"))
	bool bBarrelDistortionEnabled = true;

	//Since Barrel/Pincushion Distortion is implemented as a post process image effect, the edges of the image get cut. This setting compensates the change by adjusting the focal length, to get a similar framing. Enabled: Barrel distortion won't change your framing by compensating through focal length. Disabled: focal length will stay the same but the edges of the image get cut.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Barrel/Pincushion Distortion Focal Length Compensation enabled"))
	bool bBarrelDistortionFocalCompEnabled = false;

	//Enables slight amounts of UN-Sharpness to simulate the lenses physical limitations.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Lens Un-Sharpness enabled"))
	bool bLensSharpnessEnabled = true;

	//Enables Vignetting, a lens phenomenon that darkens the image to the edges.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Vignetting enabled"))
	bool bVignettingEnabled = true;

	//Enables Purple Fringe, a lens phenomenon, creating purple outlines around high contrast areas of the image with a more pronounced effect on the edges of the image.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Purple Fringe enabled"))
	bool bPurpleFringingEnabled = true;

	//Enables Chromatic Aberration, a lens phenomenon that splits the different color wavelengths like a prism with a more pronounced effect on the edges of the image.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Chromatic Aberration enabled"))
	bool bChromaticAberrationEnabled = true;

	//Enables Digital Film Grain.Even disabled Digital ISO Grain will still contribute to the exposure. Can not be enabled while Vintage Film Stock is in use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Camera Features", meta = (DisplayName = "Digital ISO Grain enabled"))
	bool bISOGrainEnabled = true;

private:
	virtual void PostRegisterAllComponents() override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	UPROPERTY()
	class URealCameraComponent* RealCameraComponent;

	UPROPERTY(Interp, EditAnywhere, Category = "Post Process Materials")
	UMaterial* GrainMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/Cinematographer/Ressources/RealCamera_ColoredGrain")));

	UPROPERTY()
	UMaterialInstanceDynamic* GrainDynMaterial;

	UPROPERTY(Interp, EditAnywhere, Category = "Post Process Materials")
	UMaterial* DistortionMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/Cinematographer/Ressources/RealCamera_LenseDistortion")));

	UPROPERTY()
	UMaterialInstanceDynamic* DistortionDynMaterial;

	UPROPERTY(Interp, EditAnywhere, Category = "Post Process Materials")
	UMaterial* SharpnessMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/Cinematographer/Ressources/RealCamera_LensSharpness")));

	UPROPERTY()
	UMaterialInstanceDynamic* SharpnessDynMaterial;

	UPROPERTY()
	UTextureRenderTarget2D* AverageSceneLuminanceRenderTarget = LoadObject<UTextureRenderTarget2D>(NULL, TEXT("/Cinematographer/Ressources/RealCamera_AverageSceneLuminance"));

	UPROPERTY()
	int RenderTargetSize = 32;

	UPROPERTY()
	uint32 RenderTargetTextureSizeX = RenderTargetSize;

	UPROPERTY()
	uint32 RenderTargetTextureSizeY = RenderTargetSize;

	UPROPERTY()
	class ULensFile* ActiveRealLensFile;

	UPROPERTY(VisibleDefaultsOnly, Category = "Item Config")
	class ULensComponent* RealLensComponent;

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configure Shot")
	FCameraLookatTrackingSettings RealCameraLookatTrackingSettings;


	void UpdateLookatTrackingSettings();

	//Add a Film Stock preset from our new Plugin "Vintage Film - photorealistic Film simulation filter with prestine Film Grain for 8mm, Super 8, 16mm, Super 16, 35mm, Super 35, Techniscope, 65mm, 75mmm, Kinetoscope and Cinerama
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vintage Film", meta = (DisplayName = "Film Stock"))
	UMaterialInstance* FilmMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* FilmDynMaterial;
};
