// Copyright 2022 lumines_labs. All Rights Reserved.

#pragma once

#include "RealCameraLens.h"
#include "RealCameraBody.generated.h"

UCLASS(Blueprintable)
class CINEMATOGRAPHER_API URealCameraBody : public UObject
{
	GENERATED_BODY()

private:
	URealCameraBody();

public:
	//Name of the camera model.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Sensor Characteristics", meta = (DisplayName = "Name of the camera or sensor"))
		FName ModelName = TEXT("RealCameraBody Photography Cropped Sensor Body");

	//The sensors size in mm.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Sensor Characteristics", meta = (ForceUnits = mm))
		FVector2D SensorSize = { 36.0f, 24.0f };

	//The maximum sensors resolution in pixels.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Sensor Characteristics", meta = (DisplayName = "Sensor resolution in pixels"))
		FVector2D SensorPixelResolution = { 8192,5464 };

	//The ISO range of the sensor/camera.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Sensor Characteristics")
		FIntPoint ISORange = { 100, 64000 };

	//Increase or reduce the amount of ISO grain of your sensor. 0.5 = half the amount of ISO grain. 3.0 = three times the amount of ISO grain.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Sensor Characteristics", meta = (DisplayName = "ISO Grain strength multiplier"))
		float ISOMultiplier = 1.0f;

	//The Dynamic Range describes how many EV's a sensor can capture. This number can be a bit tricky to find online but is important to simulate your cameras unique look.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Sensor Characteristics")
		float DynamicRange = 10.0f;

	//Disable for cameras which have a built in non interchangeable lens. If disabled be sure to put the build-in lens in the Built In Lens Variable below.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Built-In Lens", meta = (DisplayName = "Lens is interchangeable"))
		bool bLensIsInterchangable = true;

	//Only available, when Lens Is Interchangeable above is disabled. Enter the built-in lens, the camera should use.
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Built-In Lens", meta = (DisplayThumbnail = "true", EditCondition = "!bLensIsInterchangable"))
		TSubclassOf<URealCameraLens> BuiltInLens = nullptr;

	UPROPERTY()
		float PixelSize;

	UPROPERTY()
		int32 PixelAmount;

	UPROPERTY()
		float SensorArea;

	UPROPERTY()
		float PixelArea;
};