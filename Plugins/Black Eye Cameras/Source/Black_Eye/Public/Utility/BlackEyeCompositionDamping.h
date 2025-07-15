// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "BlackEyeCompositionDamping.generated.h"

/**
 * Struct which holds information about how our look at components damp rotations when preserving composition
 */
USTRUCT(BlueprintType)
struct BLACK_EYE_API FBlackEyeCompositionDamping
{
    GENERATED_BODY()

    FBlackEyeCompositionDamping() :
        bLinkDamping(true),
        YawDamping(0.1f),
        PitchDamping(0.1f) {}

    UPROPERTY(EditAnywhere, Interp, Category = "", meta = (ToolTip = "When enabled, Yaw damping with control both Pitch and Yaw. Disabled, they can be tuned separately"))
    bool bLinkDamping;

    UPROPERTY(EditAnywhere, Interp, Category = "", DisplayName = "(Yaw) Damping", meta = (ClampMin = 0.f, ToolTip = "Yaw damping rate, or both Yaw and Pitch when \'Link Damping\' is enabled"))
    float YawDamping;

    UPROPERTY(EditAnywhere, Interp, Category = "", meta = (ClampMin = 0.f, ToolTip = "Pitch damping rate", EditCondition = "!bLinkDamping", EditConditionHides))
    float PitchDamping;
};
