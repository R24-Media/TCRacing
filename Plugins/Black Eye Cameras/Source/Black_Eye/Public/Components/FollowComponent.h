// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

#include "Camera/CameraComponent.h"

#include "Utility/BlackEyeSimpleTarget.h"
#include "Utility/BlackEyeWeightedTarget.h"
#include "Interfaces/BlackEyeHasFollow.h"
#include "FollowComponent.generated.h"

UENUM(BlueprintType)
enum class EFollowReferenceMode : uint8 {
    FRM_World       UMETA(DisplayName = "World", ToolTip = "Use World reference frame when calculating follow position and orientation"),
    FRM_Heading     UMETA(DisplayName = "Subject Heading", ToolTip = "Use the subject's heading for Yaw, but global reference for Pitch & Roll when constructing follow position and orientation"),
    FRM_Target      UMETA(DisplayName = "Subject Locked", ToolTip = "Use the subject's rotation for reference when calculating follow position and orientation"),
};

/**
 * A component which will attempt to follow a subject, or set of subjects with a given amount of damping
 */
UCLASS(ClassGroup = "BlackEye", hideCategories = (Input, Rendering), meta = (BlueprintSpawnableComponent))
class BLACK_EYE_API UFollowComponent : public USceneComponent, public IBlackEyeHasFollow
{
    GENERATED_BODY()

public:	
    UFollowComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Use Multiple Subjects",  Category = "Follow")
    bool bUseMultipleTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow", DisplayName = "Subject", meta = (EditCondition = "!bUseMultipleTargets", EditConditionHides))
    FBlackEyeSimpleTarget Target;

    UPROPERTY(EditAnywhere, Interp, Category = "Follow", DisplayName = "Subject(s)", meta = (EditCondition = "bUseMultipleTargets", EditConditionHides))
    TArray<FBlackEyeWeightedTarget> Targets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Follow", meta = (Tooltip = "The offset to apply to this follow component relative to its Subjects(s). When following a single subject, the offset is applied in the same orientation reference as the 'Orientation Reference Mode'. When following multiple subjects, world offset will be used."))
    FVector FollowOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Follow", meta = (ClampMin = 0.f, Tooltip = "Determines the size of a sphere around the point the Follow component wants to be in. While the component's position is in this sphere no tracking will occur."))
    float FollowDwellRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Follow", meta = (EditCondition = "!bUseMultipleTargets", Tooltip = "Orientation reference mode selects how the local rotation is calculated when choosing how to damp the follow location, and rotation. Disabled when following multiple subjects"))
    EFollowReferenceMode OrientationReferenceMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Follow", DisplayName = "Location Damping", meta = (ClampMin = 0.f, Tooltip = "Sets the Follow tracking damping value for each axis of motion. This damping is applied to the orientation reference the component is currently using. Larger values make the component track slowly, and small values more quickly, i.e. 1 is quite slow, and 0.1 is quite fast."))
    FVector FollowDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Follow|Orientation", DisplayName = "Damping", meta = (EditCondition = "!bUseMultipleTargets", ClampMin = 0.f, Tooltip = "Damping when using orientation references with a single follow subject only. Larger values make the component track slowly, and small values more quickly, i.e. 1 is quite slow, and 0.1 is quite fast."))
    float OrientationDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Follow|Orientation", DisplayName = "Yaw", meta = (EditCondition = "!bUseMultipleTargets", Units = "deg", Tooltip = "Yaw offset in degrees +ve being clockwise rotation when following a single subject."))
    float YawOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Follow|Orientation", DisplayName = "Pitch", meta = (EditCondition = "!bUseMultipleTargets", Units = "deg", Tooltip = "Pitch offset in degrees +ve being clockwise rotation when following a single subject."))
    float PitchOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Follow|Orientation", DisplayName = "Roll", meta = (EditCondition = "!bUseMultipleTargets", Units = "deg", Tooltip = "Roll offset in degrees +ve being clockwise rotation when following a single subject."))
    float RollOffset;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    friend class ACameraRig;
    friend class ABlackEyeCameraRigBase;
    friend class UBlackEyeCameraSubsystem;
#endif //WITH_EDITOR

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void SnapToTargets() override;
    virtual void ClearAllTargets() override;
    virtual void SetFollow(FBlackEyeSimpleTarget Target, bool Force = false) override;

    UFUNCTION(BlueprintCallable, Category = "Follow Target")
    virtual void SetFollow(class USceneComponent* Component, FString BoneName) override;

    UFUNCTION(BlueprintCallable, Category = "Follow Target")
    virtual void SetFollowActorOverride(AActor* Actor) override;

protected:
    virtual void BeginPlay() override;

    bool GetTargetTransform(EFollowReferenceMode SingleTargetOrientationReference, 
                            bool bUseMultipleSubjects, 
                            FTransform& OutTransform, 
                            FVector& OutResolvedTargetPosition) const;

private:
    UPROPERTY(EditAnywhere, NonPIEDuplicateTransient, Category = "Hidden", meta = (HideInDetailPanel))
    FQuat DesiredRotation;
    UPROPERTY(EditAnywhere, NonPIEDuplicateTransient, Category = "Hidden", meta = (HideInDetailPanel))
    FVector DesiredPosition;
    UPROPERTY(EditAnywhere, NonPIEDuplicateTransient, Category = "Hidden", meta = (HideInDetailPanel))
    FVector TargetResolvedPosition;

    UPROPERTY(EditAnywhere, NonPIEDuplicateTransient, Category = "Hidden", meta = (HideInDetailPanel))
    FTransform TargetReferenceToWorld;

    float OrientationVelocity;

    bool SnapFollow;
    bool HasValidTargets;

#if WITH_EDITOR
    FBlackEyeSimpleTarget LastSeenFollow;

    void ProcessActorEditorTranslationDelta(const FVector& Delta);
#endif //WITH_EDITOR

    FVector FollowVelocities;


};
