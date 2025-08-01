// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlackEyeSimpleTarget.h"

#include "BlackEyeWeightedTarget.generated.h"

/**
 * A target in the black eye plugin which can be weighted for use when adjusting composition of targets
 */
USTRUCT(BlueprintType)
struct BLACK_EYE_API FBlackEyeWeightedTarget : public FBlackEyeSimpleTarget
{
    GENERATED_BODY()

    FBlackEyeWeightedTarget() :
        FBlackEyeSimpleTarget(),
        Weight(1.0f)
    {

    }

    FBlackEyeWeightedTarget(FBlackEyeSimpleTarget simpleTarget) :
        FBlackEyeSimpleTarget(simpleTarget),
        Weight(1.0f)
    {

    }

    virtual ~FBlackEyeWeightedTarget() {}

    UPROPERTY(EditAnywhere, Interp, Category = "Camera Target", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
    float Weight;

    virtual bool IsValid() const override
    {
        return Super::IsValid() && Weight > 0.f;
    }

    inline bool operator==(const FBlackEyeWeightedTarget& rhs) const
    {
        return Target == rhs.Target
            && BoneName.Equals(rhs.BoneName)
            && (FMath::Abs(Weight - rhs.Weight)) < SMALL_NUMBER;
    }

    inline bool operator!=(const FBlackEyeWeightedTarget& rhs) const
    {
        return !(Target == rhs.Target)
            || !BoneName.Equals(rhs.BoneName) 
            || (FMath::Abs(Weight - rhs.Weight)) > SMALL_NUMBER;
    }
};
