// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/SkeletalMesh.h"
#include "Runtime/Engine/Classes/Engine/SkeletalMeshSocket.h"
#include "PhysicsEngine/BodyInstance.h"

#include "CoreMinimal.h"

#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

#include "BlackEyeSimpleTarget.generated.h"

/**
 * The simplest representation of a target in the Black Eye plugin
 */
USTRUCT(BlueprintType)
struct BLACK_EYE_API FBlackEyeSimpleTarget
{
    GENERATED_BODY()

    FBlackEyeSimpleTarget() :
        Target(),
        BoneName()
    {
        FString empty = FString();
        Target.ComponentProperty = TEXT("None");
        Target.PathToComponent = empty;
        Target.OtherActor = nullptr;
        Target.OverrideComponent = nullptr;
    }

    FBlackEyeSimpleTarget(const FBlackEyeSimpleTarget& other)
    {
        Target = other.Target;
        BoneName = other.BoneName;
    }

    virtual ~FBlackEyeSimpleTarget() {}
    
public:
    UPROPERTY(EditAnywhere, Category = "Camera Target", meta = (UseComponentPicker, AllowAnyActor))
    FSoftComponentReference Target;

    UPROPERTY(EditAnywhere, Category = "Camera Target", meta = (Tooltip = "If the target is a Skeletal Mesh, use this name to bind to a bone's position when calculating the volume of this target."))
    FString BoneName;

    /**
    * Returns the position and rotation of the target component's volume (assumes use auto size)
    * @param OutPosition - The position of the center of the component's value. If none, then the component's position itself
    * @param OutRotation -  The rotation of the target component
    * @param OffsetInLocalSpace - And offset to apply to the position in the target's local space
    * @return True if the target is valid, and the position & rotation were retrieved
    */
    virtual bool GetTargetPositionAndRotation(FVector& OutPosition, FQuat& OutRotation, FVector OffsetInLocalSpace = FVector::ZeroVector) const
    {
        return GetTargetPositionAndRotation(OutPosition, OutRotation, true, 0.f, OffsetInLocalSpace);
    }

    /**
    * Returns the target component's velocity
    * @return The target's velocity. If the target is invalid, a zero vector is returned
    */
    FVector GetTargetVelocity() const
    {
        AActor* OwningActor = Target.OtherActor.Get();
        if (OwningActor == nullptr) return FVector::ZeroVector;

        const UActorComponent* TargetComponent = Target.GetComponent(OwningActor);
        if (const USkeletalMeshComponent* SkeletonComponent = Cast<USkeletalMeshComponent>(TargetComponent))
        {
            FName BoneFName = FName(BoneName);

            // Fallback to root bone if the prescribed one exists: this allows us to get its velocity as a default as the component itself does not have one
            USkeletalMesh* SkeletalMesh = SkeletonComponent->GetSkeletalMeshAsset();
            if (!SkeletonComponent->DoesSocketExist(BoneFName) && SkeletalMesh && SkeletalMesh->NumSockets() > 0)
            {
                BoneFName = SkeletalMesh->GetSocketByIndex(0)->GetFName();
            }

            if (const FBodyInstance* ParentBodyInstance = SkeletonComponent->GetBodyInstance(BoneFName))
            {
                return ParentBodyInstance->GetUnrealWorldVelocity();
            }
            else
            {
                // All else fails, return the actor's velocity here.
                return OwningActor->GetVelocity();
            }
        }
        else if (const USceneComponent* Component = Cast<USceneComponent>(TargetComponent))
        {
            // TODO: Can or should we calculate bone/socket velocities separately?
            return Component->GetComponentVelocity();
        }

        return FVector::ZeroVector;
    }

    /**
    * Returns the position of the target component's volume (assumes use auto size)
    * @param OutPosition - The position of the center of the component's value. If none, then the component's position itself
    * @param OffsetInLocalSpace - And offset to apply to the position in the target's local space
    * @return True if the target is valid, and the position & rotation were retrieved
    */
    virtual bool GetTargetPosition(FVector& OutPosition, FVector OffsetInLocalSpace = FVector::ZeroVector) const
    {
        return GetTargetPosition(OutPosition, true, 0.f, OffsetInLocalSpace);
    }

    /**
    * Whether or not the target is valid.
    * @return True if the target is valid, false otherwise
    */
    virtual bool IsValid() const
    {
        return Target.GetComponent(nullptr) != nullptr;
    }

    inline bool operator==(const FBlackEyeSimpleTarget& rhs) const
    {
        return Target == rhs.Target
            && BoneName.Equals(rhs.BoneName);
    }

    inline bool operator!=(const FBlackEyeSimpleTarget& rhs) const
    {
        return !(Target == rhs.Target)
            || !BoneName.Equals(rhs.BoneName);
    }

protected:
    bool GetTargetPositionAndRotation(FVector& OutPosition, FQuat& OutRotation, bool UseAutoSize, float ExplicitBoundingRadius, FVector OffsetInLocalSpace = FVector::ZeroVector) const
    {
        AActor* OwningActor = Target.OtherActor.Get();
        if (OwningActor == nullptr) return false;

        if (const USceneComponent* Component = Cast<USceneComponent>(Target.GetComponent(OwningActor)))
        {
            const USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Component);
            FName SocketName = FName(BoneName);
            if (SkeletalMesh != nullptr && !BoneName.IsEmpty() && SkeletalMesh->DoesSocketExist(SocketName))
            {
#if ENGINE_MINOR_VERSION < 3
                int BoneIndex = SkeletalMesh->GetBoneIndex(SocketName);
                FTransform BoneTransform = SkeletalMesh->GetBoneTransform(BoneIndex);
#else
                FTransform BoneTransform = SkeletalMesh->GetBoneTransform(SocketName);
#endif
                OutPosition = BoneTransform.TransformPosition(OffsetInLocalSpace);
                OutRotation = BoneTransform.GetRotation();
                return true;
            }

            FBox Box;
            const FTransform& SceneCompTransform = Component->GetComponentTransform();
            if (UseAutoSize && GetTargetBounds(Box, UseAutoSize, ExplicitBoundingRadius))
            {
                OutPosition = Box.GetCenter() + SceneCompTransform.TransformVector(OffsetInLocalSpace);
            }
            else
            {
                OutPosition = SceneCompTransform.TransformPosition(OffsetInLocalSpace);
            }

            OutRotation = SceneCompTransform.GetRotation();
            return true;
        }
        return false;
    }

    bool GetTargetPosition(FVector& OutPosition, bool UseAutoSize, float ExplicitBoundingRadius, FVector OffsetInLocalSpace = FVector::ZeroVector) const
    {
        AActor* OwningActor = Target.OtherActor.Get();
        if (OwningActor == nullptr) return false;

        if (const USceneComponent* Component = Cast<USceneComponent>(Target.GetComponent(OwningActor)))
        {
            FName SocketName = FName(BoneName);
            const USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Component);
            if (!UseAutoSize && SkeletalMesh != nullptr && !BoneName.IsEmpty() && SkeletalMesh->DoesSocketExist(SocketName))
            {
                FTransform SocketTransform = SkeletalMesh->GetSocketTransform(SocketName);
                OutPosition = SocketTransform.TransformPosition(OffsetInLocalSpace);
                return true;
            }

            FBox Box;
            const FTransform& SceneCompTransform = Component->GetComponentTransform();
            if (UseAutoSize && GetTargetBounds(Box, UseAutoSize, ExplicitBoundingRadius))
            {
                OutPosition = Box.GetCenter() + SceneCompTransform.TransformVector(OffsetInLocalSpace);
            }
            else
            {
                OutPosition = SceneCompTransform.TransformPosition(OffsetInLocalSpace);
            }

            return true;
        }
        return false;
    }

    bool GetTargetBounds(FBox& OutBox, bool UseAutoSize, float ExplicitBoundingRadius, bool bNonColliding = false) const
    {
        AActor* OwningActor = Target.OtherActor.Get();
        if (OwningActor == nullptr) return false;

        OutBox = OwningActor->GetComponentsBoundingBox(bNonColliding);

        if (!UseAutoSize)
        {
            // Use the center of the generated box to build our bounds.
            // TOOD: Is there a better way to get a good approximation of the center of an object without paying the cost of building its FBox?
            FVector Center;
            FQuat Rotation;
            if (!GetTargetPositionAndRotation(Center, Rotation))
            {
                Center = OutBox.GetCenter();
            }
            FVector Radius = FVector::OneVector * ExplicitBoundingRadius;
            OutBox.Min = Center - Radius;
            OutBox.Max = Center + Radius;
        }
        return true;
    }
};
