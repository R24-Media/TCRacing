// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Camera/PlayerCameraManager.h"
#include "BlackEyeCameraManager.generated.h"

/**
 *
 */
UCLASS()
class BLACK_EYE_API ABlackEyeCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

public:
    virtual void DoUpdateCamera(float DeltaTime) override;
};
