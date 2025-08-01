// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Utility/BlackEyeTarget.h"

#include "BlackEyeHasLookAt.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UBlackEyeHasLookAt : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for components which expose the ability to look at a target
 */
class BLACK_EYE_API IBlackEyeHasLookAt
{
    GENERATED_BODY()

public:
    virtual void SnapToTargets() = 0;
    virtual void ClearAllTargets() = 0;
    virtual int GetNumTargets() = 0;
    virtual int AddLookAt(FBlackEyeTarget Target, bool RequestSnap) = 0;
    virtual int AddLookAt(class USceneComponent* Component, FString BoneName, bool UseAutoSize, float BoundingRadius, bool RequestSnap) = 0;
    virtual bool RemoveTarget(int Index, bool RequestSnap) = 0;
    virtual void SetLookAt(FBlackEyeTarget Target, int Index, bool RequestSnap) = 0;
    virtual void SetLookAt(class USceneComponent* Component, FString BoneName, int Index, bool UseAutoSize, float BoundingRadius, bool RequestSnap) = 0;
    virtual void SetLookAtActorOverride(class AActor* TargetActor, int Index, bool RequestSnap) = 0;
};
