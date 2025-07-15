// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/BlackEyeCameraRigBase.h"
#include "Components/FollowComponent.h"
#include "Components/LookAtComponent.h"

#include "BlackEyeSimpleLookAt.generated.h"

/** A Black Eye camera which contains both a Follow and LookAt. Add the camera component of your choosing to use it in your scene once spawned. */
UCLASS(ClassGroup = "BlackEye", Blueprintable, BlueprintType, DisplayName = "Black Eye Simple Look At")
class ABlackEyeSimpleLookAt : public ABlackEyeCameraRigBase
{
	GENERATED_BODY()
public:
	ABlackEyeSimpleLookAt();

	UPROPERTY(BlueprintReadOnly, Category = "Follow Proeprties", VisibleDefaultsOnly, meta = (DisplayAfter = "LookAt"))
	TObjectPtr<UFollowComponent> Follow;

	UPROPERTY(BlueprintReadOnly, Category = "Look Properties", VisibleDefaultsOnly)
	TObjectPtr<ULookAtComponent> LookAt;
};
