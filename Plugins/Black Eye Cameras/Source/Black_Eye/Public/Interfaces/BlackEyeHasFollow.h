// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Utility/BlackEyeSimpleTarget.h"

#include "BlackEyeHasFollow.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UBlackEyeHasFollow : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for components which expose the ability to follow a taregt
 */
class BLACK_EYE_API IBlackEyeHasFollow
{
    GENERATED_BODY()

public:
    virtual void SnapToTargets() = 0;
    virtual void ClearAllTargets() = 0;
    virtual void SetFollow(FBlackEyeSimpleTarget Target, bool Force = false) = 0;
    virtual void SetFollow(class USceneComponent* Component, FString BoneName) = 0;
    virtual void SetFollowActorOverride(class AActor* Actor) = 0;
};
