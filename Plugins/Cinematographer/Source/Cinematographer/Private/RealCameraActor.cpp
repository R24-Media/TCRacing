// Copyright 2025 lumines_labs. All Rights Reserved.

#include "RealCameraActor.h"
#include "RealCameraComponent.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

//Auto Exposure
#include "Kismet/KismetMathLibrary.h" 

#include "Engine/World.h"
#include "Engine.h"

#if WITH_EDITOR
#include "LevelEditorViewport.h"
#endif

#include "Engine/RendererSettings.h"

class FSceneViewState;

//Init
ARealCameraActor::ARealCameraActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<URealCameraComponent>(TEXT("CameraComponent")))
{
	RealCameraComponent = Cast<URealCameraComponent>(GetCameraComponent());
	RealCameraCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("RealCameraCaptureComponent"));
	if (UGameplayStatics::GetPlatformName() == "PLATFORM_IOS" || UGameplayStatics::GetPlatformName() == "PLATFORM_ANDROID") {
		bLensSharpnessEnabled = false;
	}
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RealLensComponent = CreateDefaultSubobject<ULensComponent>(FName("RealLensComponent"));
	RealLensComponent = FindComponentByClass<ULensComponent>();
}



void ARealCameraActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	if (!bRealCameraInitialized) {
		bRealCameraInitialized = true;
		RealCameraInitilize();
	}
}

void ARealCameraActor::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(true);
}

bool ARealCameraActor::ShouldTickIfViewportsOnly() const
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor && UseEditorTick)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ARealCameraActor::RealCameraInitilize()
{
	RealCaptureInitilize();

	GrainDynMaterial = UMaterialInstanceDynamic::Create(GrainMaterial, this);
	DistortionDynMaterial = UMaterialInstanceDynamic::Create(DistortionMaterial, this);
	SharpnessDynMaterial = UMaterialInstanceDynamic::Create(SharpnessMaterial, this);
	RealCameraComponent->PostProcessSettings.AddBlendable(GrainDynMaterial, 1.0f);
	RealCameraComponent->PostProcessSettings.AddBlendable(DistortionDynMaterial, 1.0f);
	RealCameraComponent->PostProcessSettings.AddBlendable(SharpnessDynMaterial, 1.0f);

	SetActiveBody();
	SetActiveLens();
	AutoExposureInit();
}

void ARealCameraActor::RealCaptureInitilize()
{
	RealCameraCaptureComponent->AttachToComponent(RealCameraComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
	RealCameraCaptureComponent->TextureTarget = AverageSceneLuminanceRenderTarget;
	RealCameraCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalToneCurveHDR;

	RealCameraCaptureComponent->bCaptureEveryFrame = true;
	RealCameraCaptureComponent->bCaptureOnMovement = false;

	RealCameraCaptureComponent->PostProcessSettings.bOverride_AutoExposureMethod = true;
	RealCameraCaptureComponent->PostProcessSettings.AutoExposureMethod = AEM_Manual;
	RealCameraCaptureComponent->PostProcessSettings.bOverride_CameraISO = true;
	RealCameraCaptureComponent->PostProcessSettings.bOverride_AutoExposureBias = true;
	RealCameraCaptureComponent->PostProcessSettings.bOverride_CameraShutterSpeed = true;
	RealCameraCaptureComponent->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
	RealCameraCaptureComponent->PostProcessSettings.bOverride_FilmSlope = true;
	RealCameraCaptureComponent->PostProcessSettings.bOverride_FilmToe = true;
	RealCameraCaptureComponent->PostProcessSettings.bOverride_MotionBlurTargetFPS = true;
	RealCameraCaptureComponent->PostProcessSettings.bOverride_MotionBlurMax = true;
	RealCameraCaptureComponent->PostProcessSettings.MotionBlurMax = 10.0f;
	RealCameraCaptureComponent->bUseRayTracingIfEnabled = true;
}

//Tick
void ARealCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if UE_GAME || UE_SERVER || UE_EDITOR 
	if (GetWorld()->WorldType != EWorldType::Editor) {
		APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (OurPlayerController)
		{
			if ((OurPlayerController->GetViewTarget()->GetName() == this->GetName()))
			{
				if (ActiveBody && ActiveLens) {
					if (ActiveLens != ActiveRealCameraLens->GetDefaultObject<URealCameraLens>())
					{
						//UE_LOG(LogExec, Warning, TEXT("Lens Updated!"));
						SetActiveLens();
					}

					if (ActiveBody != ActiveRealCameraBody->GetDefaultObject<URealCameraBody>())
					{
						//UE_LOG(LogExec, Warning, TEXT("Body Updated!"));
						SetActiveBody();
					}
					RealCameraComponent->UpdateFocus();
					if (AutoExposureMode != EAutoExposureMode::Off) {
						RealCameraCaptureComponent->bCaptureEveryFrame = true;
						AutoExposureUpdate();
					}
					else
					{
						RealCameraCaptureComponent->bCaptureEveryFrame = false;
					}
					if (RealCameraLookatTrackingSettings.bEnableLookAtTracking || (LookatTrackingSettings.bEnableLookAtTracking && !RealCameraLookatTrackingSettings.bEnableLookAtTracking))
					{
						UpdateLookatTrackingSettings();
					}
				}
				else {
					if (!ActiveLens)
					{
						if (GEngine) { GEngine->AddOnScreenDebugMessage(4, .1, FColor::Yellow, FString::Printf(TEXT("This camera (%s) has no Lens assigned!"), *this->GetFName().ToString())); }
					}
					if (!ActiveBody)
					{
						if (GEngine) { GEngine->AddOnScreenDebugMessage(5, .1, FColor::Yellow, FString::Printf(TEXT("This camera (%s) has no CameraBody assigned!"), *this->GetFName().ToString())); }
					}
				}
			}
			else
			{
				RealCameraCaptureComponent->bCaptureEveryFrame = false;
			}
		}
	}
#endif
#if WITH_EDITOR 
	if (GetWorld()->WorldType == EWorldType::Editor) {
		auto client = (FLevelEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();
		UCameraComponent* ActiveCameraComponent = client->GetCameraComponentForView();
		ARealCameraActor* ActiveRealCameraActor = nullptr;
		if (ActiveCameraComponent)
		{
			ActiveRealCameraActor = Cast<ARealCameraActor>(ActiveCameraComponent->GetOwner());
			if (ActiveRealCameraActor)
			{
				if (ActiveRealCameraActor->GetFName() == this->GetFName())
				{
					if (ActiveBody && ActiveLens)
					{
						if (ActiveLens != ActiveRealCameraLens->GetDefaultObject<URealCameraLens>())
						{
							//UE_LOG(LogExec, Warning, TEXT("Lens Updated!"));
							SetActiveLens();
						}
						if (ActiveBody != ActiveRealCameraBody->GetDefaultObject<URealCameraBody>())
						{
							//UE_LOG(LogExec, Warning, TEXT("Body Updated!"));
							SetActiveBody();
						}
						RealCameraComponent->UpdateFocus();
						if (AutoExposureMode != EAutoExposureMode::Off) {
							RealCameraCaptureComponent->bCaptureEveryFrame = true;
							AutoExposureUpdate();
						}
						else
						{
							RealCameraCaptureComponent->bCaptureEveryFrame = false;
						}
						if (RealCameraLookatTrackingSettings.bEnableLookAtTracking || (LookatTrackingSettings.bEnableLookAtTracking && !RealCameraLookatTrackingSettings.bEnableLookAtTracking))
						{
							UpdateLookatTrackingSettings();
						}
					}
					else {
						if (!ActiveLens)
						{
							if (GEngine) { GEngine->AddOnScreenDebugMessage(4, .1, FColor::Yellow, FString::Printf(TEXT("Camera (%s) has no Lens assigned!"), *this->GetFName().ToString())); }
						}
						if (!ActiveBody)
						{
							if (GEngine) { GEngine->AddOnScreenDebugMessage(5, .1, FColor::Yellow, FString::Printf(TEXT("Camera (%s) has no CameraBody assigned!"), *this->GetFName().ToString())); }
						}
					}
				}
				else
				{
					RealCameraCaptureComponent->bCaptureEveryFrame = false;
				}
			}
			else
			{
				RealCameraCaptureComponent->bCaptureEveryFrame = false;
			}
		}
		else
		{
			RealCameraCaptureComponent->bCaptureEveryFrame = false;
		}
	}
#endif
}

//AutoExposure
void ARealCameraActor::AutoExposureInit()
{
	AverageSceneLuminanceRenderTarget->MipLoadOptions = ETextureMipLoadOptions::OnlyFirstMip;
	AverageSceneLuminanceRenderTarget->LODGroup = TextureGroup::TEXTUREGROUP_UI;

	UpdateAspectRatioRenderTexture();

	for (int i = ISOs.Num() - 1; CurrentISORange.X >= ISOs[i] && i > 0; i--) {
		EVISORangeMinMax.Y = i;
	}

	int32 EVISORangeMax = 0;
	while (ISOs[EVISORangeMax] > CurrentISORange.Y) {
		EVISORangeMax++;
	}
	EVISORangeMinMax.X = EVISORangeMax;
	EVISORange = EVISORangeMinMax.Y - EVISORangeMinMax.X;
	EVISOBasePosition = EVISOCompensationPosition = EVISORange;

	CurrentISO = ISOs[EVISORangeMinMax.X + EVISOBasePosition];

	for (int i = FStops.Num() - 1; CurrentApertureRange.Y <= FStops[i]; i--) {
		EVApertureRangeMinMax.Y = i;
	}
	int32 EVApertureRangeMin = 0;
	while (FStops[EVApertureRangeMin] < CurrentApertureRange.X) {
		EVApertureRangeMin++;
	}
	EVApertureRangeMinMax.X = EVApertureRangeMin;
	EVApertureRange = EVApertureRangeMinMax.Y - EVApertureRangeMinMax.X;

	if (FilmMaterial && FilmDynMaterial)
	{
		AutoExposureMode = EAutoExposureMode::ShutterPriority;
		RealCameraShutterSettings.CameraMode = ERealCameraMode::Movie;
		
		if (FilmMaterial && FilmDynMaterial->K2_GetScalarParameterValue(FName("Framerate"))) {
			RealCameraShutterSettings.TargetFPS = FilmDynMaterial->K2_GetScalarParameterValue(FName("Framerate"));
			RealCameraShutterSettings.bTragetFPSVisible = false;
		}
	}
	else { RealCameraShutterSettings.bTragetFPSVisible = true;
		RealCameraShutterSettings.TargetFPS = 24;
	}

	if (AutoExposureMode == EAutoExposureMode::FullAutomatic || AutoExposureMode == EAutoExposureMode::AperturePriority) 
	{
		RealCameraShutterSettings.CameraMode = ERealCameraMode::Photography;
		RealCameraShutterSettings.CurrentShutterSpeed = 4000;
		EVShutterBasePosition = EVShutterCompensationPosition = EVShutterRange;
	}
	if (AutoExposureMode == EAutoExposureMode::ShutterPriority) {
		int32 ShutterPositionCheck = 0;

		if (RealCameraShutterSettings.CameraMode == ERealCameraMode::Photography){ ShutterPositionCheck = RealCameraShutterSettings.CurrentShutterSpeed;}
		if (RealCameraShutterSettings.CameraMode == ERealCameraMode::Movie){ ShutterPositionCheck = RealCameraShutterSettings.TargetFPS;}
		if (RealCameraShutterSettings.CameraMode == ERealCameraMode::Game){ ShutterPositionCheck = RealCameraShutterSettings.SimulatedFPS;}

		int32 CurrentShutterPosition = 0;
		while (Shutterspeeds[CurrentShutterPosition] < ShutterPositionCheck) {
			CurrentShutterPosition++;
		}
		EVShutterBasePosition = EVShutterCompensationPosition = CurrentShutterPosition;
	}

	if (AutoExposureMode == EAutoExposureMode::FullAutomatic || AutoExposureMode == EAutoExposureMode::ShutterPriority)
	{
		EVApertureBasePosition = EVApertureCompensationPosition = EVApertureRange;
		if (ActiveLens) {
			CurrentAperture = CurrentApertureRange.Y;
		}
	}

	if (AutoExposureMode == EAutoExposureMode::AperturePriority)
	{
		int32 AperturePositionCheck = CurrentAperture;
		int32 CurrentAperturePosition = 0;
		while (FStops[CurrentAperturePosition] < CurrentAperture) {
			CurrentAperturePosition++;
		}
		EVApertureBasePosition = EVApertureCompensationPosition = CurrentAperturePosition;
	}

	EVShutterRange = Shutterspeeds.Num() - 1;

	EVShutterRangeMinMax.X = 0;
	EVShutterRangeMinMax.Y = Shutterspeeds.Num() - 1;
	EVRange = EVShutterRange + EVISORange + EVApertureRange;
	AverageSceneLuminanceRenderTarget->InitAutoFormat(RenderTargetSize, RenderTargetSize);

	if (AutoExposureMode == EAutoExposureMode::FullAutomatic)
	{
		BaseEV = ((EVISORangeMinMax.X + EVISOBasePosition) - EV100ISOPoisiton) + (EVShutterRangeMinMax.Y - EV100ShutterPoisiton) + (EVApertureRangeMinMax.Y - EV100AperturePoisiton);
	}
	if (AutoExposureMode == EAutoExposureMode::ShutterPriority)
	{
		BaseEV = ((EVISORangeMinMax.X + EVISOBasePosition) - EV100ISOPoisiton) + (EVShutterBasePosition - EV100ShutterPoisiton) + (EVApertureRangeMinMax.Y - EV100AperturePoisiton);
	}
	if (AutoExposureMode == EAutoExposureMode::AperturePriority)
	{
		BaseEV = ((EVISORangeMinMax.X + EVISOBasePosition) - EV100ISOPoisiton) + (EVShutterRangeMinMax.Y - EV100ShutterPoisiton) + (EVApertureBasePosition - EV100AperturePoisiton);
	}
	CurrentEV = BaseEV;

	UpdateShutter();
	UpdateAperture();
	UpdateISO();
}

void ARealCameraActor::AutoExposureUpdate()
{
	TArray<FColor> SurfData;
	FRenderTarget* RenderTarget = AverageSceneLuminanceRenderTarget->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(SurfData);

	FColor PixelColor = { 0,0,0,0 };
	uint32 R = 0;
	uint32 G = 0;
	uint32 B = 0;
	//SurfData.Num();
	float LuminanceCollector = 0.0f;

	uint32 RenderTargetPixels = 0;

	if (RenderTargetTextureSizeY < 1) RenderTargetTextureSizeY = 1;
	if (RenderTargetTextureSizeX < 1) RenderTargetTextureSizeX = 1;

	for (uint32 y = (RenderTargetSize - RenderTargetTextureSizeY) / 2; y < RenderTargetSize - (RenderTargetSize - RenderTargetTextureSizeY) / 2; y++)
	{	/* Debug Console Preview 01
		FString DebugPixel = "";
		*/

		for (uint32 x = (RenderTargetSize - RenderTargetTextureSizeX) / 2; x < RenderTargetSize - (RenderTargetSize - RenderTargetTextureSizeX) / 2; x++)
		{

			R += SurfData[y * RenderTargetSize + x].R;
			G += SurfData[y * RenderTargetSize + x].G;
			B += SurfData[y * RenderTargetSize + x].B;
			RenderTargetPixels++;
			/* Debug Console Preview
			uint32 TR = SurfData[y* RenderTargetSize + x].R;
			uint32 TG = SurfData[y * RenderTargetSize + x].G;
			uint32 TB = SurfData[y * RenderTargetSize + x].B;
			FLinearColor TAverageSceneValue = FColor{uint8(TR),uint8(TG),uint8(TB),0}.ReinterpretAsLinear();
			float TAverageLuminance = TAverageSceneValue.GetLuminance();

			if (TAverageLuminance >= 1.0f) {
				DebugPixel.Append(TEXT("@ "));
			}
			if (TAverageLuminance > 0.75) {
				DebugPixel.Append(TEXT("# "));
			}
			else if (TAverageLuminance > 0.5) {
				DebugPixel.Append(TEXT("+ "));
			}
			else if (TAverageLuminance > 0.25) {

				DebugPixel.Append(TEXT("- "));
			}
			else if (TAverageLuminance < 0.25 && TAverageLuminance > 0.0f)
			{
				DebugPixel.Append(TEXT(". "));
			}
			else
			{
				DebugPixel.Append(TEXT("  "));
			}
			*/

		}
		/* Debug Console Preview 02
		UE_LOG(LogExec, Warning, TEXT("%s"),*DebugPixel);
		*/
	}
	RenderTargetPixels--;

	PixelColor.R = R / RenderTargetPixels;
	PixelColor.G = G / RenderTargetPixels;
	PixelColor.B = B / RenderTargetPixels;

	FLinearColor AverageSceneValue = PixelColor.ReinterpretAsLinear();
	float AverageLuminance = AverageSceneValue.GetLuminance();

	CompensationEV = BaseEV + (int(AutoExposureCompensationSteps) - 9);


	if (GetDefault<URendererSettings>()->bEnableRayTracing != 0)
	{
		AutoExposureLuminanceTarget = AutoExposureLuminanceTargetLumen;
	}
	else
	{
		AutoExposureLuminanceTarget = AutoExposureLuminanceTargetLegacy;
	}

	//EV Luminance Check
	bAutoExposureUpdatedThisTick = false;
	bIsCompensationUpdate = false;
	if (AverageLuminance > FMath::Clamp(float(AutoExposureLuminanceTarget + AutoExposureLuminanceThreshold), 0.0f, 1.0f))
	{
		StopDown(EVISOBasePosition, EVShutterBasePosition, EVApertureBasePosition);
	}
	else if (AverageLuminance < FMath::Clamp(float(AutoExposureLuminanceTarget - AutoExposureLuminanceThreshold), 0.0f, 1.0f))
	{
		StopUp(EVISOBasePosition, EVShutterBasePosition, EVApertureBasePosition);
	}

	if (CurrentEV < CompensationEV && !bAutoExposureUpdatedThisTick) 
	{
		bIsCompensationUpdate = true;
		StopDown(EVISOCompensationPosition, EVShutterCompensationPosition, EVApertureCompensationPosition);
	}
	else if (CurrentEV > CompensationEV && !bAutoExposureUpdatedThisTick) 
	{
		bIsCompensationUpdate = true;
		StopUp(EVISOCompensationPosition, EVShutterCompensationPosition, EVApertureCompensationPosition);
	}
}

void ARealCameraActor::StopUp(int& ISOPosition, int& EVShutterPosition, int& AperturePosition)
{
	if (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVApertureRange > 0 && EVISORange > 0 && EVShutterRange > 0)
	{
		if (EVShutterPosition > EVShutterRange * 0.5f ||
			AperturePosition > EVApertureRange * 0.5f)
		{
			if (UKismetMathLibrary::SafeDivide(1.0f, (EVShutterRange)) * EVShutterPosition >= UKismetMathLibrary::SafeDivide(1.0f, (EVApertureRange)) * AperturePosition)
			{
				ShutterDecrease();
			}
			else
			{
				ApertureDecrease();
			}
		}
		else if (EVShutterPosition > 1 || AperturePosition > 1)
		{
			if ((UKismetMathLibrary::SafeDivide(1.0f, (EVISORange - (EVISORange *0.5f))) * (ISOPosition - (EVISORange * 0.5f)) > UKismetMathLibrary::SafeDivide(1.0f, float(EVShutterRange * 0.5f)) * float(EVShutterPosition * 0.5f)) ||
				(UKismetMathLibrary::SafeDivide(1.0f, (EVISORange - (EVISORange * 0.5f))) * (ISOPosition - (EVISORange * 0.5f)) > UKismetMathLibrary::SafeDivide(1.0f, float(EVApertureRange * 0.5f)) * float(AperturePosition *  0.5f)))
			{
				ISODecrease();
			}
			else if (UKismetMathLibrary::SafeDivide(1.0f, (EVShutterRange)) * EVShutterPosition >= UKismetMathLibrary::SafeDivide(1.0f, (EVApertureRange)) * AperturePosition)
			{
				ShutterDecrease();
			}
			else
			{
				ApertureDecrease();
			}
		}
		else if (AperturePosition == 1)
		{
			ApertureDecrease();
		}
		else if (EVShutterPosition == 1)
		{
			ShutterDecrease();
		}
		else if (ISOPosition > 0)
		{
			ISODecrease();
		}
	}
	if ((AutoExposureMode == EAutoExposureMode::AperturePriority && EVISORange > 0 && EVShutterRange > 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVApertureRange == 0 && EVISORange > 0 && EVShutterRange > 0))
	{
		if (EVShutterPosition > EVShutterRange * 0.5f)
		{
				ShutterDecrease();	
		} 
		else if (EVShutterPosition > 1)
		{
			if ((UKismetMathLibrary::SafeDivide(1.0f, (EVISORange - (EVISORange * 0.5f))) * (ISOPosition - (EVISORange * 0.5f)) > UKismetMathLibrary::SafeDivide(1.0f, float(EVShutterRange * 0.5f)) * float(EVShutterPosition * 0.5f)))
			{
				ISODecrease();
			}
			else 
			{
				ShutterDecrease();
			}
		}
		else if (EVShutterPosition == 1)
		{
			ShutterDecrease();
		}
		else if (ISOPosition > 0)
		{
			ISODecrease();
		}
	}
	if ((AutoExposureMode == EAutoExposureMode::ShutterPriority && EVApertureRange > 0 && EVISORange > 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVShutterRange == 0 && EVApertureRange > 0 && EVISORange))
	{
		if (AperturePosition > EVApertureRange * 0.5f)
		{		
				ApertureDecrease();	
		} 
		else if (AperturePosition > 1)
		{
			if ((UKismetMathLibrary::SafeDivide(1.0f, (EVISORange - (EVISORange * 0.5f))) * (ISOPosition - (EVISORange * 0.5f)) > UKismetMathLibrary::SafeDivide(1.0f, float(EVApertureRange * 0.5f)) * float(AperturePosition * 0.5f)))
			{
				ISODecrease();
			}
			else
			{
				ApertureDecrease();
			}
		}
		else if (AperturePosition == 1)
		{
			ApertureDecrease();
		}
		else if (ISOPosition > 0)
		{
			ISODecrease();
		}
	}
	if ((AutoExposureMode == EAutoExposureMode::FullAutomatic && EVShutterRange > 0 && EVApertureRange > 0 && EVISORange == 0))
	{
		if (UKismetMathLibrary::SafeDivide(1.0f, (EVShutterRange)) * EVShutterPosition >= UKismetMathLibrary::SafeDivide(1.0f, (EVApertureRange)) * AperturePosition)
		{
			if (EVShutterPosition > 0)
			{
				ShutterDecrease();
			}
		}
		else
		{
			if (AperturePosition > 0)
			{
				ApertureDecrease();
			}
		}
	}

	if ((AutoExposureMode == EAutoExposureMode::ShutterPriority && EVApertureRange > 0 && EVISORange == 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVShutterRange == 0 && EVISORange == 0 && EVApertureRange > 0) )
	{
		if (AperturePosition > 0)
		{
			ApertureDecrease();
		}
		
	}

	if ((AutoExposureMode == EAutoExposureMode::AperturePriority && EVShutterRange > 0 && EVISORange == 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic &&  EVApertureRange == 0 && EVISORange == 0 && EVShutterRange > 0))
	{
		if (EVShutterPosition > 0)
		{
			ShutterDecrease();
		}
	}



	if ((AutoExposureMode == EAutoExposureMode::ShutterPriority && EVApertureRange == 0 && EVISORange > 0) || (AutoExposureMode == EAutoExposureMode::AperturePriority && EVShutterRange == 0 && EVISORange > 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVShutterRange == 0 && EVApertureRange == 0 && EVISORange > 0))
	{
		if (ISOPosition > 0)
		{
			ISODecrease();
		}
	}
}

void ARealCameraActor::StopDown(int& ISOPosition, int& EVShutterPosition, int& AperturePosition)
{
	if (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVApertureRange > 0 && EVISORange > 0 && EVShutterRange > 0)
	{
		if (ISOPosition < EVISORange * 0.5f)
		{
			ISOIncrease();
		}
		else if (EVShutterPosition == 1)
		{
			ShutterIncrease();
		}
		else if (AperturePosition == 1)
		{
			ApertureIncrease();
		}
		else if (EVShutterPosition < EVShutterRange * 0.5f && AperturePosition < EVApertureRange * 0.5f)
		{
			if ((UKismetMathLibrary::SafeDivide(1.0f, (EVISORange * 0.5f)) * float(ISOPosition  * 0.5f) < UKismetMathLibrary::SafeDivide(1.0f, float(EVShutterRange * 0.5f)) * float(EVShutterPosition)) ||
				(UKismetMathLibrary::SafeDivide(1.0f, (EVISORange * 0.5f)) * float(ISOPosition  * 0.5f) < UKismetMathLibrary::SafeDivide(1.0f, float(EVApertureRange * 0.5f)) * float(AperturePosition)))
			{
				ISOIncrease();
			}
			else if (UKismetMathLibrary::SafeDivide(1.0f, (EVShutterRange)) * EVShutterPosition <= UKismetMathLibrary::SafeDivide(1.0f, (EVApertureRange)) * AperturePosition)
			{
				ShutterIncrease();
			}
			else
			{
				ApertureIncrease();
			}
		}
		else if (EVShutterPosition < EVShutterRange || AperturePosition < EVApertureRange)
		{
			if (UKismetMathLibrary::SafeDivide(1.0f, (EVShutterRange)) * EVShutterPosition <= UKismetMathLibrary::SafeDivide(1.0f, (EVApertureRange)) * AperturePosition)
			{
				ShutterIncrease();
			}
			else
			{
				ApertureIncrease();
			}
		}
	}
	if ((AutoExposureMode == EAutoExposureMode::AperturePriority && EVISORange > 0 && EVShutterRange > 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVApertureRange == 0 && EVISORange > 0 && EVShutterRange > 0))
	{
		if (ISOPosition < EVISORange * 0.5f)
		{
			ISOIncrease();
		}
		else if (EVShutterPosition == 1)
		{
			ShutterIncrease();
		}
		else if (EVShutterPosition < EVShutterRange * 0.5f)
		{
			if ((UKismetMathLibrary::SafeDivide(1.0f, (EVISORange * 0.5f)) * float(ISOPosition * 0.5f) < UKismetMathLibrary::SafeDivide(1.0f, float(EVShutterRange * 0.5f)) * float(EVShutterPosition)))
			{
				ISOIncrease();
			}
			else
			{
				ShutterIncrease();
			}
		}
		else if (EVShutterPosition < EVShutterRange)
		{
				ShutterIncrease();
		}
	}
	if ((AutoExposureMode == EAutoExposureMode::ShutterPriority && EVApertureRange > 0 && EVISORange > 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVShutterRange == 0 && EVApertureRange > 0 && EVISORange > 0))
	{
		if (ISOPosition < EVISORange * 0.5f)
		{
			ISOIncrease();
		}
		else if (AperturePosition == 1)
		{
			ApertureIncrease();
		}
		else if (AperturePosition < EVApertureRange * 0.5f)
		{
			if ((UKismetMathLibrary::SafeDivide(1.0f, (EVISORange * 0.5f)) * float(ISOPosition * 0.5f) < UKismetMathLibrary::SafeDivide(1.0f, float(EVApertureRange * 0.5f)) * float(AperturePosition)))
			{
				ISOIncrease();
			}
			else
			{
				ApertureIncrease();
			}
		}
		else if (AperturePosition < EVApertureRange)
		{
				ApertureIncrease();
		}
	}

	if ((AutoExposureMode == EAutoExposureMode::FullAutomatic && EVShutterRange > 0 && EVApertureRange > 0 && EVISORange == 0))
	{
		if (EVShutterPosition < EVShutterRange || AperturePosition < EVApertureRange)
		{
			if (UKismetMathLibrary::SafeDivide(1.0f, (EVShutterRange)) * EVShutterPosition <= UKismetMathLibrary::SafeDivide(1.0f, (EVApertureRange)) * AperturePosition)
			{
				ShutterIncrease();
			}
			else
			{
				ApertureIncrease();
			}
		}
	}

	if ((AutoExposureMode == EAutoExposureMode::ShutterPriority && EVApertureRange > 0 && EVISORange == 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVShutterRange == 0 && EVISORange == 0 && EVApertureRange > 0))
	{

		if (AperturePosition < EVApertureRange)
		{
			ApertureIncrease();
		}
		
	}

	if ((AutoExposureMode == EAutoExposureMode::AperturePriority && EVShutterRange > 0 && EVISORange == 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVApertureRange == 0 && EVISORange == 0 && EVShutterRange > 0))
	{
		if (EVShutterPosition < EVShutterRange)
		{
			ShutterIncrease();
		}
	}




	if ((AutoExposureMode == EAutoExposureMode::ShutterPriority && EVApertureRange == 0 && EVISORange > 0) || (AutoExposureMode == EAutoExposureMode::AperturePriority && EVShutterRange == 0 && EVISORange > 0) || (AutoExposureMode == EAutoExposureMode::FullAutomatic && EVShutterRange == 0 && EVApertureRange == 0 && EVISORange > 0))
	{
		if (ISOPosition < EVISORange)
		{
			ISOIncrease();
		}
	}
}

void ARealCameraActor::ISOIncrease() {
	if (!bIsCompensationUpdate)
	{
		bAutoExposureUpdatedThisTick = true;
		EVISOBasePosition++;
		BaseEV += 1.0f;
	}
	if (EVISOCompensationPosition < EVISORange)
	{
		bAutoExposureUpdatedThisTick = true;
		EVISOCompensationPosition++;
		CurrentEV += 1.0f;
	}
	CurrentISO = ISOs[EVISORangeMinMax.X + EVISOCompensationPosition];
	UpdateISO();
}
void ARealCameraActor::ISODecrease()
{
	if (!bIsCompensationUpdate)
	{
		bAutoExposureUpdatedThisTick = true;
		EVISOBasePosition--;
		BaseEV -= 1.0f;
	}
	if (EVISOCompensationPosition > 0)
	{
		bAutoExposureUpdatedThisTick = true;
		EVISOCompensationPosition--;
		CurrentEV -= 1.0f;
	}

	CurrentISO = ISOs[EVISORangeMinMax.X + EVISOCompensationPosition];
	UpdateISO();
}
void ARealCameraActor::ShutterIncrease() {

	if (!bIsCompensationUpdate)
	{
		bAutoExposureUpdatedThisTick = true;
		EVShutterBasePosition++;
		BaseEV += 1.0f;
	}
	if (EVShutterCompensationPosition < EVShutterRange) {
		bAutoExposureUpdatedThisTick = true;
		EVShutterCompensationPosition++;
		CurrentEV += 1.0f;
	}
	RealCameraShutterSettings.CurrentShutterSpeed = Shutterspeeds[EVShutterRangeMinMax.X + EVShutterCompensationPosition];
	UpdateShutter();
}
void ARealCameraActor::ShutterDecrease() {
	if (!bIsCompensationUpdate)
	{
		bAutoExposureUpdatedThisTick = true;
		EVShutterBasePosition--;
		BaseEV -= 1.0f;
	}
	if (EVShutterCompensationPosition > 0)
	{
		bAutoExposureUpdatedThisTick = true;
		EVShutterCompensationPosition--;
		CurrentEV -= 1.0f;
	}
	RealCameraShutterSettings.CurrentShutterSpeed = Shutterspeeds[EVShutterCompensationPosition];
	UpdateShutter();
}
void ARealCameraActor::ApertureIncrease() {
	if (!bIsCompensationUpdate)
	{
		bAutoExposureUpdatedThisTick = true;
		EVApertureBasePosition++;
		BaseEV += 1.0f;
	}
	if (EVApertureCompensationPosition < EVApertureRange) {
		bAutoExposureUpdatedThisTick = true;
		EVApertureCompensationPosition++;
		CurrentEV += 1.0f;
	}
	CurrentAperture = FStops[EVApertureRangeMinMax.X + EVApertureCompensationPosition];
	UpdateAperture();
}
void ARealCameraActor::ApertureDecrease() {
	if (!bIsCompensationUpdate)
	{
		bAutoExposureUpdatedThisTick = true;
		EVApertureBasePosition--;
		BaseEV -= 1.0f;
	}
	if (EVApertureCompensationPosition > 0) {
		bAutoExposureUpdatedThisTick = true;
		EVApertureCompensationPosition--;
		CurrentEV -= 1.0f;
	}
	CurrentAperture = FStops[EVApertureRangeMinMax.X + EVApertureCompensationPosition];
	UpdateAperture();
}


#if WITH_EDITOR
void ARealCameraActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyChangedName = PropertyChangedEvent.GetPropertyName();
	if (PropertyChangedEvent.MemberProperty->GetFName() == "CustomAspectRatio")
	{
		UpdateAspectRatio();
	}
	if (PropertyChangedEvent.MemberProperty->GetFName() == "ActiveRealCameraLens" && !ActiveRealCameraLens)
	{
		ActiveLens = nullptr;
		RealLensUpdate();
	}
	if (PropertyChangedEvent.MemberProperty->GetFName() == "ActiveRealCameraBody" && !ActiveRealCameraBody)
	{
		ActiveBody = nullptr;
		RealBodyUpdate();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ARealCameraActor::UpdateAutoExposure() {

	if (bRealCameraInitialized) {

		switch (AutoExposureMode) {
		case EAutoExposureMode::FullAutomatic:
			AutoExposureInit();
			break;
		case EAutoExposureMode::AperturePriority:
			AutoExposureInit();
			break;
		case EAutoExposureMode::ShutterPriority:
			AutoExposureInit();
			break;
		case EAutoExposureMode::Off:
			break;
		default:
			break;
		}
	}
}

void ARealCameraActor::SetActiveBody()
{
	if (ActiveRealCameraBody)
	{
		ActiveBody = ActiveRealCameraBody->GetDefaultObject<URealCameraBody>();
		RealBodyUpdate();
		if (ActiveBody && !ActiveBody->bLensIsInterchangable)
		{
			if (ActiveBody->BuiltInLens)
			{
				ActiveLens = ActiveBody->BuiltInLens->GetDefaultObject<URealCameraLens>();
				ActiveRealCameraLens = ActiveBody->BuiltInLens;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s has no Lens assigned!"), *ActiveBody->GetFName().ToString());
				ActiveLens = nullptr;
				ActiveRealCameraLens = nullptr;
			}
			RealLensUpdate();
			bCurrentLensIsInterchangable = ActiveBody->bLensIsInterchangable;
		}
		else if (ActiveBody && ActiveBody->bLensIsInterchangable)
		{
			bCurrentLensIsInterchangable = ActiveBody->bLensIsInterchangable;
		}
	}
}

void ARealCameraActor::SetActiveLens()
{
	if (ActiveRealCameraLens)
	{
		ActiveLens = ActiveRealCameraLens->GetDefaultObject<URealCameraLens>();
		RealLensUpdate();
	}
}

void ARealCameraActor::RealLensUpdate()
{
	if (bRealCameraInitialized) {
		if (RealCameraComponent && ActiveLens) {
			ActiveRealCameraLensName = ActiveLens->LenseName;
			if (ActiveLens->b_isPrimeLens)
			{
				ActiveFocalLength = ActiveLens->FocalLength;
				bIsPrimeLens = true;
			}
			else
			{
				ActiveFocalLength = ActiveLens->FocalLength;
				bIsPrimeLens = false;
			}
			UpdateLensFile();
			UpdateFocalLength();
			//UpdateAperture();
			UpdateApertureRange();
			UpdateBarrelDistortion();
			UpdateVignetting();
			UpdatePurpleFringing();
			UpdateChromaticAberration();
			UpdateDoFBlades();
			UpdateLensSharpness();
			RealCameraComponent->UpdateFocus();
		}
		else if (!ActiveLens)
		{
			ActiveLens = nullptr;
			ActiveRealCameraLens = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("%s has no Lens assigned!"), *GetFName().ToString());
		}
		if (AutoExposureMode != EAutoExposureMode::Off) {
			AutoExposureInit();
		}
		else {
			UpdateExposure();
		}
		UpdateAperture();
	}
}

void ARealCameraActor::RealBodyUpdate()
{
	if (bRealCameraInitialized) 
	{
		if (ActiveBody != nullptr)
		{
			UpdateSensorSize();
			UpdateSimulatedDynamicRange();
			UpdateAspectRatio();
			UpdateISORange();
			UpdateISO();
		}
		else if (!ActiveBody)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has no Body assigned!"), *GetFName().ToString());
		}
		RealLensUpdate();
		if (AutoExposureMode != EAutoExposureMode::Off) 
		{
			AutoExposureInit();
		}
		else 
		{
			UpdateExposure();
		}
	}
}

void ARealCameraActor::UpdateShutter()
{
	if (RealCameraComponent)
	{
		if (RealCameraShutterSettings.CameraMode == ERealCameraMode::Photography)
		{
			RealCameraComponent->PostProcessSettings.MotionBlurTargetFPS = RealCameraShutterSettings.CurrentShutterSpeed;
			if (bMotionBlurEnabled)
			{
				RealCameraComponent->PostProcessSettings.MotionBlurAmount = 1.f;
			}
			else 
			{
				RealCameraComponent->PostProcessSettings.MotionBlurAmount = 0.f;
			}
			ShutterExposureCompensation = 0;

			RealCameraComponent->PostProcessSettings.CameraShutterSpeed = RealCameraShutterSettings.CurrentShutterSpeed;
			if (AutoExposureMode == EAutoExposureMode::FullAutomatic || AutoExposureMode == EAutoExposureMode::AperturePriority) {
				if (!bIsCompensationUpdate)
				{
					RealCameraCaptureComponent->PostProcessSettings.CameraShutterSpeed = Shutterspeeds[EVShutterRangeMinMax.X + EVShutterBasePosition];
				}
			}
			else
			{
				RealCameraCaptureComponent->PostProcessSettings.CameraShutterSpeed = RealCameraShutterSettings.CurrentShutterSpeed;
			}
		}
		else if (RealCameraShutterSettings.CameraMode == ERealCameraMode::Movie)
		{
			RealCameraComponent->PostProcessSettings.MotionBlurTargetFPS = RealCameraShutterSettings.TargetFPS;
			if (bMotionBlurEnabled)
			{
				RealCameraComponent->PostProcessSettings.MotionBlurAmount = 1.f / (360.0f / RealCameraShutterSettings.CurrentShutterAngle);
			}
			else
			{
				RealCameraComponent->PostProcessSettings.MotionBlurAmount = 0.f;
			}
			ShutterExposureCompensation = (360.0f / RealCameraShutterSettings.CurrentShutterAngle) - 1.f;
			RealCameraComponent->PostProcessSettings.CameraShutterSpeed = RealCameraShutterSettings.TargetFPS;

			if (AutoExposureMode == EAutoExposureMode::FullAutomatic || AutoExposureMode == EAutoExposureMode::AperturePriority) {
				if (!bIsCompensationUpdate)
				{
					RealCameraCaptureComponent->PostProcessSettings.CameraShutterSpeed = RealCameraComponent->PostProcessSettings.CameraShutterSpeed;
				}
			}
			else
			{
				RealCameraCaptureComponent->PostProcessSettings.CameraShutterSpeed = RealCameraComponent->PostProcessSettings.CameraShutterSpeed;
			}
		}
		else if (RealCameraShutterSettings.CameraMode == ERealCameraMode::Game)
		{
			if (bMotionBlurEnabled)
			{
				RealCameraComponent->PostProcessSettings.MotionBlurTargetFPS = RealCameraShutterSettings.SimulatedFPS;
				RealCameraComponent->PostProcessSettings.MotionBlurAmount = 1.f / (360.0f / RealCameraShutterSettings.CurrentShutterAngle);
			}
			else
			{
				RealCameraComponent->PostProcessSettings.MotionBlurAmount = 0.f;
			}
			ShutterExposureCompensation = (360.0f / RealCameraShutterSettings.CurrentShutterAngle) - 1.f;
			RealCameraComponent->PostProcessSettings.CameraShutterSpeed = RealCameraShutterSettings.SimulatedFPS;
			if (AutoExposureMode == EAutoExposureMode::FullAutomatic || AutoExposureMode == EAutoExposureMode::AperturePriority) {
				if (!bIsCompensationUpdate)
				{
					RealCameraCaptureComponent->PostProcessSettings.CameraShutterSpeed = RealCameraComponent->PostProcessSettings.CameraShutterSpeed;
				}
			}
			else
			{
				RealCameraCaptureComponent->PostProcessSettings.CameraShutterSpeed = RealCameraComponent->PostProcessSettings.CameraShutterSpeed;
			}
		}
		UpdateExposure();
	}
}

void ARealCameraActor::UpdateAperture()
{
	if (RealCameraComponent && ActiveLens)
	{ 
		CurrentAperture = FMath::Clamp(CurrentAperture, ActiveLens->FStopRange.X, ActiveLens->FStopRange.Y);
		RealCameraComponent->CurrentAperture = CurrentAperture;
		RealCameraComponent->PostProcessSettings.DepthOfFieldFstop = FStops[EVApertureRangeMinMax.X + EVApertureBasePosition];
		if (!bIsCompensationUpdate) RealCameraCaptureComponent->PostProcessSettings.DepthOfFieldFstop = CurrentAperture;
	}
}

void ARealCameraActor::UpdateApertureRange()
{
	if (RealCameraComponent && ActiveLens)
	{
		CurrentApertureRange.X = ActiveLens->FStopRange.X;
		CurrentApertureRange.Y = ActiveLens->FStopRange.Y;

		if (CurrentApertureRange.X < FStops[0]) {
			CurrentApertureRange.X = FStops[0];
		}
		if (CurrentApertureRange.Y > FStops[FStops.Num() - 1]) {
			CurrentApertureRange.Y = FStops[FStops.Num() - 1];
		}
	}
}

void ARealCameraActor::UpdateFocalLength()
{
	if (ActiveLens && RealCameraComponent)
	{
		if (ActiveLens->b_isPrimeLens && !FilmMaterial)
		{
			ActiveFocalLength = ActiveLens->FocalLength;
		}
		else
		{
			ActiveFocalLength = FMath::Clamp(ActiveFocalLength, ActiveLens->FocalLengthRange.X, ActiveLens->FocalLengthRange.Y);
		}
		if (bBarrelDistortionEnabled && bBarrelDistortionFocalCompEnabled)
		{
			if (ActiveLens->BarrelDistortion > 0.f)
			{
				FocalLenghtCompensation = ((ActiveFocalLength / 100.f) * 12.93f) * (sin((ActiveLens->BarrelDistortion * PI) / 2.f)); 
			}
			else
			{	
				FocalLenghtCompensation = ((ActiveFocalLength / 100.f) * 72.5f) * (sin((abs(ActiveLens->BarrelDistortion) * PI) / 2.f));
			}		
		}
		else
		{
			FocalLenghtCompensation = .0f;
		}
		if (ActiveLens->b_isFullFrameEquivalent) 
		{
			RealCameraComponent->SetCurrentFocalLength((ActiveFocalLength - FocalLenghtCompensation) / CurrentCropFactor);
			RealCameraCaptureComponent->FOVAngle = RealCameraComponent->FieldOfView;
		}
		else
		{
			RealCameraComponent->SetCurrentFocalLength(ActiveFocalLength - FocalLenghtCompensation);
			RealCameraCaptureComponent->FOVAngle = RealCameraComponent->FieldOfView;
		}	
	}
}

void ARealCameraActor::UpdateISO()
{
	if (RealCameraComponent)
	{	

		if (FilmMaterial) {
			
			bISOGrainEnabled = false;
			RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight = 0.0f;
		}
		else if (!FilmMaterial) {
			RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight = 1.0f;
		}

		if (AutoExposureMode == EAutoExposureMode::Off) {
			CurrentISO = FMath::Clamp(CurrentISO, CurrentISORange.X, CurrentISORange.Y);
			RealCameraComponent->PostProcessSettings.CameraISO = CurrentISO;
		}
		else{
			RealCameraComponent->PostProcessSettings.CameraISO = FMath::Clamp(ISOs[EVISORangeMinMax.X + EVISOCompensationPosition], CurrentISORange.X, CurrentISORange.Y);
			if (!bIsCompensationUpdate) RealCameraCaptureComponent->PostProcessSettings.CameraISO = FMath::Clamp(ISOs[EVISORangeMinMax.X + EVISOBasePosition], CurrentISORange.X, CurrentISORange.Y);
		}
		if (bISOGrainEnabled && ActiveLens && ActiveBody)
		{
			float Grain = ((1.f / 51200 * (CurrentISO * (CurrentCropFactor * CurrentCropFactor))) * ISO_Max) * CurrentISOMultiplier;
			if (bISOGrainEnabled)
				ISO_Max = 0.55f;
			else ISO_Max = 0.0f;

			if (GrainMaterial && !GrainDynMaterial)
			{
				GrainDynMaterial = UMaterialInstanceDynamic::Create(GrainMaterial, this);
			}
			if (GrainMaterial && GrainDynMaterial ) {
				GrainDynMaterial->SetScalarParameterValue("RealCamera_FilmGrain_Blend", Grain);
				RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Object = GrainDynMaterial;
				RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight = 1.0f;
			}
			UpdateExposure();
		}
		else
		{
			if (!FilmMaterial) {
				if (GrainMaterial && !GrainDynMaterial)
				{
					GrainDynMaterial = UMaterialInstanceDynamic::Create(GrainMaterial, this);
				}
				if (GrainMaterial && GrainDynMaterial) {
					GrainDynMaterial->SetScalarParameterValue("RealCamera_FilmGrain_Blend", .0f);
					RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Object = GrainDynMaterial;
					RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight = 1.0f;
				}
			}
		}
	}
}

void ARealCameraActor::UpdateExposure()
{
	if (RealCameraComponent)
	{
		if (ActiveLens && ActiveBody)
		{
			if (bExposureCompensationEnabled)
			{
				RealCameraComponent->PostProcessSettings.AutoExposureBias = -CurrentNDFilter - ShutterExposureCompensation;
				RealCameraCaptureComponent->PostProcessSettings.AutoExposureBias = -CurrentNDFilter - ShutterExposureCompensation;
			}
			else
			{
				RealCameraComponent->PostProcessSettings.AutoExposureBias = 0.f;
                RealCameraCaptureComponent->PostProcessSettings.AutoExposureBias = 0.f;
			}
		}
		else if (!ActiveLens || !ActiveBody)
		{
			RealCameraComponent->PostProcessSettings.AutoExposureBias = -100.0f;
		}
	}
}

void ARealCameraActor::UpdateAutoExposureCompensation() {}

void ARealCameraActor::UpdateSimulatedDynamicRange()
{
	if (ActiveBody && RealCameraComponent)
	{
		if (FilmMaterial)
		{
			bSimulatedDynamicRangeEnabled = false;
		}
		if (bSimulatedDynamicRangeEnabled)
		{
			float DynamicRangePercent = (100 - ((ActiveBody->DynamicRange - 8.5) * 2.33))/100;
			RealCameraComponent->PostProcessSettings.FilmSlope = 0.88 * DynamicRangePercent;
			RealCameraComponent->PostProcessSettings.FilmToe = 0.55 * DynamicRangePercent;
			if (RealCameraCaptureComponent)
			{
				RealCameraCaptureComponent->PostProcessSettings.FilmSlope = 0.88 * DynamicRangePercent;
				RealCameraCaptureComponent->PostProcessSettings.FilmToe = 0.55 * DynamicRangePercent;
			}
		}
		else 
		{
			RealCameraComponent->PostProcessSettings.FilmSlope = 0.88;
			RealCameraComponent->PostProcessSettings.FilmToe = 0.55;
			if (RealCameraCaptureComponent)
			{
				RealCameraCaptureComponent->PostProcessSettings.FilmSlope = 0.88;
				RealCameraCaptureComponent->PostProcessSettings.FilmToe = 0.55;
			}
		}
	}
}

void ARealCameraActor::UpdateAspectRatio()
{
	UpdateAspectRatioRenderTexture();

	if (RealCameraComponent && ActiveBody){
		if (FilmDynMaterial && FilmMaterial) {
			float CustomAspectRatioFilmX = FilmDynMaterial->K2_GetScalarParameterValue(FName("AspectRatioX"));
			float CustomAspectRatioFilmY = FilmDynMaterial->K2_GetScalarParameterValue(FName("AspectRatioY"));

			if (!bCustomAspectRatio) {
				if ((FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm")) / FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm"))) < (CustomAspectRatioFilmX / CustomAspectRatioFilmY))
				{
					RealCameraComponent->Filmback.SensorWidth = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm")); 
					RealCameraComponent->Filmback.SensorHeight = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm")) / (CustomAspectRatioFilmX / CustomAspectRatioFilmY);
				}
				else
				{
					RealCameraComponent->Filmback.SensorWidth = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm")) * (CustomAspectRatioFilmX / CustomAspectRatioFilmY);
					RealCameraComponent->Filmback.SensorHeight = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm"));
				}
			}
			if (bCustomAspectRatio) {
				if ((FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm")) / FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm"))) < (CustomAspectRatio.X / CustomAspectRatio.Y))
				{
					RealCameraComponent->Filmback.SensorHeight = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm")) / (CustomAspectRatio.X / CustomAspectRatio.Y);
					RealCameraComponent->Filmback.SensorWidth = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm"));
				}
				else
				{
					RealCameraComponent->Filmback.SensorWidth = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm")) * (CustomAspectRatio.X / CustomAspectRatio.Y);
					RealCameraComponent->Filmback.SensorHeight = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm"));
				}
			}
		}
		else if (bCustomAspectRatio){
	
			if ((ActiveBody->SensorSize.X / ActiveBody->SensorSize.Y) < (CustomAspectRatio.X / CustomAspectRatio.Y))
			{
				RealCameraComponent->Filmback.SensorHeight = ActiveBody->SensorSize.X / ( CustomAspectRatio.X / CustomAspectRatio.Y);
				RealCameraComponent->Filmback.SensorWidth = ActiveBody->SensorSize.X;
			}
			else
			{
				RealCameraComponent->Filmback.SensorWidth = ActiveBody->SensorSize.Y * (CustomAspectRatio.X / CustomAspectRatio.Y);
				RealCameraComponent->Filmback.SensorHeight = ActiveBody->SensorSize.Y;
			}
		}
		else
		{
			RealCameraComponent->Filmback.SensorHeight = ActiveBody->SensorSize.Y;
			RealCameraComponent->Filmback.SensorWidth = ActiveBody->SensorSize.X;
		}
	}
}

void ARealCameraActor::UpdateAspectRatioRenderTexture()
{
	if (AverageSceneLuminanceRenderTarget && ActiveBody && CustomAspectRatio.X > 0.0f && CustomAspectRatio.Y > 0.0f) {
		if (bCustomAspectRatio) {
			if ((CustomAspectRatio.X > CustomAspectRatio.Y)) {

				RenderTargetTextureSizeX = RenderTargetSize;
				RenderTargetTextureSizeY = int(UKismetMathLibrary::SafeDivide(RenderTargetSize, CustomAspectRatio.X) * CustomAspectRatio.Y);
			}
			else {
				RenderTargetTextureSizeX = int(UKismetMathLibrary::SafeDivide(RenderTargetSize, CustomAspectRatio.Y) * CustomAspectRatio.X);
				RenderTargetTextureSizeY = RenderTargetSize;
			}
		}
		else if ( (ActiveBody->SensorSize.X > ActiveBody->SensorSize.Y))
		{
			RenderTargetTextureSizeX = RenderTargetSize;
			RenderTargetTextureSizeY = int(UKismetMathLibrary::SafeDivide(RenderTargetSize, (ActiveBody->SensorSize.X)) * ActiveBody->SensorSize.Y);
		}
		else
		{
			RenderTargetTextureSizeX = int(UKismetMathLibrary::SafeDivide(RenderTargetSize, (ActiveBody->SensorSize.Y)) * ActiveBody->SensorSize.X);
			RenderTargetTextureSizeY = RenderTargetSize;
		}
	}
}

void ARealCameraActor::UpdateBarrelDistortion()
{
	if (ActiveLens && RealCameraComponent){
		if (ActiveLens->BarrelDistortion >= 0.0f) {
			CurrentBarrelDistortion = ActiveLens->BarrelDistortion;
		}
		else {
			CurrentBarrelDistortion = ActiveLens->BarrelDistortion / 1.538f;
		}
		UpdateFocalLength();
		if (DistortionMaterial && !DistortionDynMaterial)
		{
			DistortionDynMaterial = UMaterialInstanceDynamic::Create(DistortionMaterial, this);
		}
		if (DistortionDynMaterial) {
			DistortionDynMaterial->SetScalarParameterValue("RealCamera_Distortion_Intensity", CurrentBarrelDistortion);
			DistortionDynMaterial->SetScalarParameterValue("RealCamera_PurpleFringe_Intensity", CurrentPurpleFringing);
			RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[1].Object = DistortionDynMaterial;

			if (bBarrelDistortionEnabled && !ActiveLens->b_useLensFile)
			{	
				RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[1].Weight = 1.f;
			}
			else 
			{
				RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[1].Weight = 0.f;
			}
		}
	}
}

void ARealCameraActor::UpdateLensSharpness()
{
	if (ActiveLens && RealCameraComponent){		
		CurrentGeneralLensSharpness = ActiveLens->GeneralLensSharpness;
		CurrentCornerLensSharpness = ActiveLens->CornerLensSharpness;

		if (SharpnessMaterial && !SharpnessDynMaterial)
		{
			SharpnessDynMaterial = UMaterialInstanceDynamic::Create(SharpnessMaterial, this);
		}
		if (SharpnessDynMaterial) {
			SharpnessDynMaterial->SetScalarParameterValue("RealCamera_LensBlur_Intensity", CurrentGeneralLensSharpness);
			SharpnessDynMaterial->SetScalarParameterValue("RealCamera_Sharpness_CornerBlur_Intensity", CurrentCornerLensSharpness);
			RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[2].Object = SharpnessDynMaterial;

			if (UGameplayStatics::GetPlatformName() == "PLATFORM_IOS" || UGameplayStatics::GetPlatformName() == "PLATFORM_ANDROID") {
				UE_LOG(LogTemp, Warning, TEXT("Platform Mobile: Disabled Lens Un-Sharpness"));
				bLensSharpnessEnabled = false;
			}
			if (bLensSharpnessEnabled)
			{
				RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[2].Weight = 1.f;
			}
			else
			{
				RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[2].Weight = 0.f;
			}
		}
	}
}

void ARealCameraActor::UpdateVignetting()
{
	if (ActiveLens && RealCameraComponent)
	{
		if (bVignettingEnabled)
		{
			CurrentVignetting = ActiveLens->Vignetting;
			RealCameraComponent->PostProcessSettings.VignetteIntensity = CurrentVignetting;
		}
		else RealCameraComponent->PostProcessSettings.VignetteIntensity = 0.f;
	}
}

void ARealCameraActor::UpdatePurpleFringing()
{
	if (ActiveLens && RealCameraComponent)
	{
		if (bPurpleFringingEnabled) {
			CurrentPurpleFringing = ActiveLens->PurpleFringing;
		}
		else
		{
			CurrentPurpleFringing = 0.f;
		}
		if (DistortionMaterial && !DistortionDynMaterial)
		{
			DistortionDynMaterial = UMaterialInstanceDynamic::Create(DistortionMaterial, this);
		}
		if (DistortionDynMaterial) {
			DistortionDynMaterial->SetScalarParameterValue("RealCamera_Distortion_Intensity", CurrentBarrelDistortion);
			DistortionDynMaterial->SetScalarParameterValue("RealCamera_PurpleFringe_Intensity", CurrentPurpleFringing);
		}
	}
}

void ARealCameraActor::UpdateChromaticAberration()
{
	if (ActiveLens && RealCameraComponent)
	{
		if (bChromaticAberrationEnabled)
		{
			CurrentChromaticAberration = ActiveLens->ChromaticAberration;
			RealCameraComponent->PostProcessSettings.SceneFringeIntensity = CurrentChromaticAberration;
		}
		else RealCameraComponent->PostProcessSettings.SceneFringeIntensity = 0.f;
	}
}

void ARealCameraActor::UpdateDoFBlades()
{
	if (ActiveLens && RealCameraComponent)
	{
		CurrentBladeCount = ActiveLens->NumberOfBlades;
		RealCameraComponent->LensSettings.DiaphragmBladeCount = CurrentBladeCount;
	}
}

void ARealCameraActor::UpdateISORange()
{
	if (ActiveBody && RealCameraComponent)
	{
		if (FilmDynMaterial && FilmMaterial) {
			if (!FilmDynMaterial->K2_GetScalarParameterValue(FName("EI/ISO"))) {
				
			}
			else if (FilmMaterial && FilmDynMaterial->K2_GetScalarParameterValue(FName("EI/ISO"))) {
				int CustomISO = FilmDynMaterial->K2_GetScalarParameterValue(FName("EI/ISO"));
				CurrentISORange.X = CustomISO;
				CurrentISORange.Y = CustomISO;
			}
		}
		else {
			CurrentISORange = ActiveBody->ISORange;
		}

		if (CurrentISORange.Y > ISOs[0]) {
			CurrentISORange.Y = ISOs[0];
		}
		if (CurrentISORange.X < ISOs[ISOs.Num() - 1]) {
			CurrentISORange.X = ISOs[ISOs.Num() - 1];
		}
		CurrentISOMultiplier = ActiveBody->ISOMultiplier;
		UpdateISO();
	}
}

void ARealCameraActor::UpdateSensorSize()
{
	if (ActiveBody && RealCameraComponent)
	{
		if (FilmMaterial && FilmDynMaterial) {
			ActiveBody->SensorArea = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm")) * FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm"));
			
			RealCameraComponent->Filmback.SensorWidth = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm"));
			RealCameraComponent->Filmback.SensorHeight = FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm"));
			CurrentCropFactor = 43.27f / sqrt((FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm")) * FilmDynMaterial->K2_GetScalarParameterValue(FName("Film width in mm"))) + (FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm")) * FilmDynMaterial->K2_GetScalarParameterValue(FName("Film height in mm"))));
		}
		else{
		ActiveBody->SensorArea = ActiveBody->SensorSize.X * ActiveBody->SensorSize.Y;
		RealCameraComponent->Filmback.SensorWidth = ActiveBody->SensorSize.X;
		RealCameraComponent->Filmback.SensorHeight = ActiveBody->SensorSize.Y;
		CurrentCropFactor = 43.27f / sqrt((ActiveBody->SensorSize.X * ActiveBody->SensorSize.X) + (ActiveBody->SensorSize.Y * ActiveBody->SensorSize.Y));
		
		}
		ActiveBody->PixelAmount = ActiveBody->SensorPixelResolution.X * ActiveBody->SensorPixelResolution.Y;
		ActiveBody->PixelSize = ActiveBody->SensorArea / ActiveBody->PixelAmount;
		CurrentPixelSize = ActiveBody->PixelSize;
		
	}
}

void ARealCameraActor::SetActiveFocalLength(float value)
{
	ActiveFocalLength = value;
	UpdateFocalLength();
}

void ARealCameraActor::SetCurrentAperture(float value)
{
	CurrentAperture = value;
	UpdateAperture();
}

void ARealCameraActor::SetCurrentISO(int32 value)
{
	CurrentISO = value;
	UpdateISO();
}

void ARealCameraActor::SetCurrentNDFilter(float value)
{
	CurrentNDFilter = value;
	UpdateExposure();
}

void ARealCameraActor::SetCustomAspectRatio(FVector2D value)
{
	CustomAspectRatio = value;
	UpdateAspectRatio();
}

void ARealCameraActor::UpdateLensFile() 
{
	if (ActiveLens->b_useLensFile)
	{
		bCurrentLensFileInUse = true;
		RealLensComponent->SetLensFile(ActiveLens->RealLensFile);
		RealLensComponent->SetFIZEvaluationMode(EFIZEvaluationMode::UseCameraSettings);
		RealLensComponent->SetApplyDistortion(true);
	}
	else
	{
		bCurrentLensFileInUse = false;
		RealLensComponent->SetLensFile(nullptr);
		RealLensComponent->SetApplyDistortion(false);
	}
}

void ARealCameraActor::UpdateLookatTrackingSettings()
{
	LookatTrackingSettings = RealCameraLookatTrackingSettings;
}

void ARealCameraActor::UpdateFilm()
{
	if (FilmMaterial && !FilmDynMaterial)
	{
		FilmDynMaterial = UMaterialInstanceDynamic::Create(FilmMaterial, this);
		RealCameraComponent->PostProcessSettings.AddBlendable(FilmMaterial, 1.0f);
	}
	if (FilmMaterial && FilmDynMaterial) 
	{
		FilmDynMaterial = UMaterialInstanceDynamic::Create(FilmMaterial, this);
		RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[3].Object = FilmDynMaterial;
		RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[3].Weight = 1.0f;
		GrainDynMaterial->SetScalarParameterValue("RealCamera_FilmGrain_Blend", .0f);
	}
	else if (!FilmMaterial) 
	{
		bSimulatedDynamicRangeEnabled = true;
		bISOGrainEnabled = true;
		RealCameraComponent->PostProcessSettings.WeightedBlendables.Array[3].Weight = 0.0f;
		RealBodyUpdate();
	}
	RealBodyUpdate();
}

#if WITH_EDITOR
bool ARealCameraActor::CanEditChange(const FProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ARealCameraActor, ActiveRealCameraLensName))
	{
		if (ActiveBody && ActiveBody->BuiltInLens)
		{
			return false;
		}
		return (ParentVal);
	}
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ARealCameraActor,CurrentAperture))
	{
		return (ParentVal && AutoExposureMode == EAutoExposureMode::Off && EVApertureRange > 0) || (ParentVal && AutoExposureMode == EAutoExposureMode::AperturePriority && EVApertureRange > 0);
	}
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ARealCameraActor, CurrentISO))
	{
		return ParentVal && AutoExposureMode == EAutoExposureMode::Off && !FilmMaterial;
	}
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraShutterSettings.CameraMode))
	{
		return ParentVal && AutoExposureMode == EAutoExposureMode::Off || ParentVal && AutoExposureMode == EAutoExposureMode::ShutterPriority;
	}
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ARealCameraActor, RealCameraShutterSettings))
	{
		if (AutoExposureMode == EAutoExposureMode::ShutterPriority || AutoExposureMode == EAutoExposureMode::Off) {
				return true;
		
		} else return false;
	}
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ARealCameraActor, AutoExposureCompensationSteps))
	{
		return ParentVal && AutoExposureMode != EAutoExposureMode::Off;
	}
	
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ARealCameraActor, ActiveFocalLength))
	{
		return ParentVal && !bIsPrimeLens;
	}
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ARealCameraActor, bBarrelDistortionEnabled))
	{
		return ParentVal && !bCurrentLensFileInUse;
	}
	return ParentVal;
}
#endif