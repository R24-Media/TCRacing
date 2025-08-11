// Copyright 2022 lumines_labs. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CineCameraComponent.h"
#include "RealCameraActor.h"
#include "RealCameraComponent.generated.h"


UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class CINEMATOGRAPHER_API URealCameraComponent : public UCineCameraComponent
{
	GENERATED_BODY()

public:
	URealCameraComponent();

	//Default Unreal Engine Focus Settings
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Focus", meta = (DisplayName = "Focus Settings"))
	FCameraFocusSettings RealCameraFocusSettings;

	UPROPERTY()
	class ARealCameraActor* RealCameraActor;

	void UpdateFocus();

protected:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};
