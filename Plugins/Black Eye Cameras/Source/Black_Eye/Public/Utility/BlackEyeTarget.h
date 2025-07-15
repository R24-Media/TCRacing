// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "Utility/BlackEyeSimpleTarget.h"
#include "BlackEyeTarget.generated.h"

/**
 * A target in the Black Eye plugin which can also have a size associated with it
 */
USTRUCT(BlueprintType)
struct BLACK_EYE_API FBlackEyeTarget : public FBlackEyeSimpleTarget {
    GENERATED_BODY()

    FBlackEyeTarget() :
        FBlackEyeSimpleTarget(),
        bAutoSize(true),
        BoundingRadius(10.0f)
    {
    }

public:
    UPROPERTY(EditAnywhere, Category = "Camera Target", meta = (DisplayName = "Use Actor Bounds", Tooltip = "Automatically calculate the volume and target point for the actor"))
    bool bAutoSize;

    UPROPERTY(EditAnywhere, Category = "Camera Target", meta = (EditCondition = "!bAutoSize", EditConditionHides))
    float BoundingRadius;

    /**
    * Gets the bounding radius of this target. By automatically interrogating the actor's bounds, or the explicit size as defined in this target
    * @return The size of this target's bounding radius in world units
    */
    float GetBoundingRadius() const
    {
        AActor* OwningActor = Target.OtherActor.Get();
        if (OwningActor == nullptr) return 0.f;

        if (bAutoSize)
        {
            FVector ActorOrigin, ActorExtents;
            OwningActor->GetActorBounds(true, ActorOrigin, ActorExtents);

            return ActorExtents.GetMax();
        }

        return BoundingRadius;
    }

    /**
    * Gets the Bound Box of this target. By automatically interrogating the actor's bounds, or the explicit size as defined in this target
    * @param OutBox - The out parameter where the calculated box will be stored
    * @param bNonColliding - True if non colliding components are used to calculate the volume
    * @return True if the target is valid and bounds were calculated. False otherwise
    */
    bool GetTargetBounds(FBox& OutBox, bool bNonColliding = false) const
    {
        return Super::GetTargetBounds(OutBox, bAutoSize, BoundingRadius, bNonColliding);
    }

    /**
    * Returns the position and rotation of the target component's volume (assumes use auto size)
    * @param OutPosition - The position of the center of the component's value. If none, then the component's position itself
    * @param OutRotation -  The rotation of the target component
    * @param OffsetInLocalSpace - And offset to apply to the position in the target's local space
    * @return True if the target is valid, and the position & rotation were retrieved
    */
    virtual bool GetTargetPositionAndRotation(FVector& OutPosition, FQuat& OutRotation, FVector OffsetInLocalSpace = FVector::ZeroVector) const override
    {
        return Super::GetTargetPositionAndRotation(OutPosition, OutRotation, bAutoSize, BoundingRadius, OffsetInLocalSpace);
    }

    /**
    * Returns the position of the target component's volume (assumes use auto size)
    * @param OutPosition - The position of the center of the component's value. If none, then the component's position itself
    * @param OffsetInLocalSpace - And offset to apply to the position in the target's local space
    * @return True if the target is valid, and the position & rotation were retrieved
    */
    bool GetTargetPosition(FVector& OutPosition, FVector OffsetInLocalSpace = FVector::ZeroVector) const override
    {
        return Super::GetTargetPosition(OutPosition, bAutoSize, BoundingRadius, OffsetInLocalSpace);
    }

    inline bool operator==(const FBlackEyeTarget& rhs) const
    {
        return Target == rhs.Target
            && BoneName.Equals(rhs.BoneName)
            && bAutoSize == rhs.bAutoSize
            && (FMath::Abs(BoundingRadius - rhs.BoundingRadius)) < SMALL_NUMBER;
    }

    inline bool operator!=(const FBlackEyeTarget& rhs) const
    {
        return !(Target == rhs.Target)
            || !BoneName.Equals(rhs.BoneName) 
            || bAutoSize != rhs.bAutoSize
            || (FMath::Abs(BoundingRadius - rhs.BoundingRadius)) > SMALL_NUMBER;
    }
};
