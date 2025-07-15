// Copyright 2022 lumines_labs. All Rights Reserved.

#pragma once

#include "CineCameraComponent.h"
#include "RealCameraLens.generated.h"

UCLASS(Blueprintable)
class CINEMATOGRAPHER_API URealCameraLens : public UObject
{
	GENERATED_BODY()

public:
	URealCameraLens();

	//Name of the lens.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics")
		FName LenseName = TEXT("RealLens Photography Lens 18-135mm f/3.5-5.6 ");

	//Enable if the lens has only one set focal length and can not zoom.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (DisplayName = "Is it a Prime Lens?"))
		bool b_isPrimeLens = false;

	//Is the given focal length equivalent to a full frame sensor or is it the focal length of the cameras sensors size? In most cases the focal length of a lens is given as a full frame equivalent.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (DisplayName = "Is the focal length full frame equivalent?"))
		bool b_isFullFrameEquivalent = false;

	//The focal length of the prime lens or the default focal length for a zoom lens.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (ForceUnits = mm))
		float FocalLength = 19.0f;

	//X = the lowest focal length of this lens in mm. Y = the highest focal length of this lens in mm. Not used when the lens is a prime lens.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (ForceUnits = mm, EditCondition = "!b_IsPrimeLens"))
		FVector2D FocalLengthRange { 19.0f, 135.0f};

	//X = the lowest f-stop of this lens. Y = the highest f-stop of this lens. If the lens has only one fixed aperture enter it at X and Y.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics")
		FVector2D FStopRange { 3.5f, 22.0f};

	//Minimum distance this lens can focus on in cm.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (ForceUnits = cm, DisplayName = "Minimum focus distance"))
		float FocusDistanceMin = 15.f;

	//The number of aperture blades the lens has.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (DisplayName = "Number of aperture blades"))
		uint8 NumberOfBlades = 11;

	//An arbitrary value from 0.0 - 1.0. Use positiv values for barrel distortion and negativ values for pincushion distortion. Never use 0.0 since every lens has flaws!
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (DisplayName = "Barrel/Pincushion Distortion"))
		float BarrelDistortion = 0.15f;

	//This value describes how UN-sharp the lens is. An arbitrary value from 0.0 - 1.0. Never use 0.0 since every lens has flaws!
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (DisplayName = "Lens Un-Sharpness"))
		float GeneralLensSharpness = 0.3f;

	//This value adds additional UN-sharpness to the corners of the lens. An arbitrary value from 0.0 - 1.0. Never use 0.0 since every lens has flaws!
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics", meta = (DisplayName = "Lens Corner Un-Sharpness"))
		float CornerLensSharpness = 0.3f;

	//Vignetting is a lens phenomenon that darkens the image to the edges. An arbitrary value from 0.0 - 1.0. Never use 0.0 since every lens has flaws!
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics")
		float Vignetting = 0.35f;

	//Purple Fringing is a lens phenomenon, creating purple outlines around high contrast areas of the image with a more pronounced effect on the edges of the image. An arbitrary value from 0.0 - 1.0. Never use 0.0 since every Lens has flaws!
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics")
		float PurpleFringing = 0.05f;

	//Chromatic Aberration is a lens phenomenon that splits the different color wavelengths like a prism with a more pronounced effect on the edges of the image. An arbitrary value from 0.0 - 1.0. Never use 0.0 since every lens has flaws!
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Lens Characteristics")
		float ChromaticAberration = 0.35f;

	UPROPERTY()
		FCameraLensSettings RealLensSettings;
};