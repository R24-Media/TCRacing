// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "Components/FollowComponent.h"

#include "BlackEyeCVars.h"

#include "GameFramework/Actor.h"

#include "Interfaces/BlackEyeHasLookAt.h"

#include "Utility/BlackEyeMath.h"

DECLARE_CYCLE_STAT(TEXT("Follow Component Tick"), STAT_FollowComponent, STATGROUP_Game);

UFollowComponent::UFollowComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostPhysics;

    this->Target = FBlackEyeSimpleTarget();

    this->FollowOffset = FVector::ZeroVector;

    this->FollowDamping = FVector::ZeroVector;
    this->FollowDwellRadius = 0.f;

    this->OrientationReferenceMode = EFollowReferenceMode::FRM_World;
    this->OrientationDamping = 0.f;

    this->OrientationVelocity = 0.f;
}

void UFollowComponent::BeginPlay()
{
    Super::BeginPlay();

    //This fixes any stale or un set look ats for child rig components so they don't "boing" in play mode
    this->SetFollow(this->Target, true);

    this->FollowVelocities = FVector::ZeroVector;
    this->OrientationVelocity = 0.f;
    this->DesiredRotation = this->GetComponentQuat();

    this->TargetReferenceToWorld = this->GetComponentTransform();
}

bool UFollowComponent::GetTargetTransform(EFollowReferenceMode SingleTargetOrientationReference, 
                                            bool bUseMultipleSubjects,
                                            FTransform& OutTransform, 
                                            FVector& OutResolvedTargetPosition) const
{
    const FTransform& ComponentTransform = this->GetComponentTransform();
    FQuat CurrentRotation = ComponentTransform.GetRotation();
    FQuat TargetRotation = CurrentRotation;

    FVector ForwardVector = FVector::ForwardVector;
    FVector UpVector = FVector::UpVector;

    int ValidTargetCount = 0;
    if (this->bUseMultipleTargets)
    {
        FVector PositionAccumulator = FVector::ZeroVector;
        float ValidTargetWeightSum = 0.f;
        for (int i = 0; i < this->Targets.Num(); ++i)
        {
            if (this->Targets[i].IsValid())
            {
                ValidTargetWeightSum += this->Targets[i].Weight;
            }
        }

        for (int i = 0; i < this->Targets.Num(); ++i)
        {
            if (this->Targets[i].IsValid())
            {
                FVector Pos;
                // Do nothing with rotaiton
                this->Targets[i].GetTargetPosition(Pos);
                PositionAccumulator += Pos * (this->Targets[i].Weight / ValidTargetWeightSum);
                ValidTargetCount++;
            }
        }

        if (ValidTargetCount > 0)
        {
            OutResolvedTargetPosition = PositionAccumulator;
        }
    }
    else if (this->Target.IsValid())
    {
        ValidTargetCount++;
        Target.GetTargetPositionAndRotation(OutResolvedTargetPosition, TargetRotation);

        switch (SingleTargetOrientationReference)
        {
        case EFollowReferenceMode::FRM_World:
            UpVector = FVector::UpVector;
            ForwardVector = FVector::ForwardVector;
            break;

        case EFollowReferenceMode::FRM_Heading:
            UpVector = FVector::UpVector;
            ForwardVector = TargetRotation.GetForwardVector();
            ForwardVector.Z = 0.f;

            // If the projection onto the up-facing plane is near zero, use the local up vector projected onto it instead
            if (ForwardVector.SquaredLength() <= UE_KINDA_SMALL_NUMBER)
            {
                ForwardVector = TargetRotation.GetUpVector();
                ForwardVector.Z = 0.f;
            }
            ForwardVector.Normalize();
            break;

        case EFollowReferenceMode::FRM_Target:
            ForwardVector = TargetRotation.GetForwardVector();
            UpVector = TargetRotation.GetUpVector();
            break;
        }
    }

    // Construct a transform from our resolved rotation and target for reference when damping our follow
    FQuat ResolvedOrientation = FBlackEyeMath::MakeLookAt(ForwardVector, UpVector);
    OutTransform = FTransform(ResolvedOrientation.Rotator(), OutResolvedTargetPosition, FVector::OneVector);

    return ValidTargetCount > 0;
}

void UFollowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    SCOPE_CYCLE_COUNTER(STAT_FollowComponent);

    bool ForceLookChange = false;
    bool ForceFollowChange = false;
#if WITH_EDITOR
    ForceFollowChange = this->Target != this->LastSeenFollow;
#endif

    //TODO: Clean this up: shouldn't have to do it each tick
    this->SetFollow(this->Target, ForceFollowChange);

    this->SnapFollow |= FBlackEyeCVars::DampingDisabled();

    const FTransform& ComponentTransform = this->GetComponentTransform();
    FVector ComponentLocation = ComponentTransform.GetLocation();
    FQuat CurrentRotation = ComponentTransform.GetRotation();

    this->TargetResolvedPosition = ComponentLocation;
    HasValidTargets = GetTargetTransform(this->OrientationReferenceMode, this->bUseMultipleTargets, TargetReferenceToWorld, this->TargetResolvedPosition);

    // Get the position relative to the target and our constructed orientation space
    FVector CurrentLocalPosition = TargetReferenceToWorld.InverseTransformPositionNoScale(ComponentLocation);
    FVector DesiredLocalPosition = this->HasValidTargets ? this->FollowOffset : FVector::ZeroVector;
    FVector DesiredWorldPosition = TargetReferenceToWorld.TransformPositionNoScale(DesiredLocalPosition);

    // If we have specified a dwell radius based on the result of our follow subjects, manipulate our desired endpoint by stopping motion, 
    // or project it onto th edge of the sphere
    if (this->FollowDwellRadius > 0.f)
    {
        FVector FromDesiredToCurrentVectorLocal = (CurrentLocalPosition - DesiredLocalPosition);
        float DistanceSqFromDesiredLocal = FromDesiredToCurrentVectorLocal.SquaredLength();
        if (DistanceSqFromDesiredLocal < this->FollowDwellRadius * this->FollowDwellRadius)
        {
            DesiredLocalPosition = CurrentLocalPosition;
            this->FollowVelocities = FVector::ZeroVector;
        }
        else
        {
            FromDesiredToCurrentVectorLocal.Normalize();
            DesiredLocalPosition += FromDesiredToCurrentVectorLocal * this->FollowDwellRadius;
        }
    }

    FVector DampedLocalPosition = FBlackEyeMath::SmoothDamp(CurrentLocalPosition, DesiredLocalPosition, this->FollowVelocities, this->FollowDamping, DeltaTime);

    // Bring the position back to world space
    FVector DampedWorldPosition = TargetReferenceToWorld.TransformPositionNoScale(DampedLocalPosition);

    DesiredPosition = DesiredWorldPosition;

    this->SetWorldLocation(SnapFollow ? DesiredWorldPosition : DampedWorldPosition);

    if (!this->bUseMultipleTargets && HasValidTargets)
    {
        // Calculate our orientation as well, including our additional offsets if we're using a single target and it's valid
        FRotator ResolvedRotation = TargetReferenceToWorld.GetRotation().Rotator();
        FRotator AdditionalRotation(this->PitchOffset, this->YawOffset, this->RollOffset);
        DesiredRotation = ResolvedRotation.Quaternion() * AdditionalRotation.Quaternion();

        FQuat DampedRotation = FBlackEyeMath::SmoothDamp(CurrentRotation, 
                                                        DesiredRotation, 
                                                        this->OrientationVelocity, 
                                                        this->OrientationDamping, 
                                                        DeltaTime);
        
        this->SetWorldRotation(SnapFollow ? DesiredRotation : DampedRotation);
    }
    else
    {
        DesiredRotation = CurrentRotation;
    }

    if (this->SnapFollow)
    {
        this->SnapFollow = false;
        this->FollowVelocities = FVector::ZeroVector;
        this->OrientationVelocity = 0.f;
    }
}

void UFollowComponent::SnapToTargets()
{
    this->SnapFollow = true;
}

void UFollowComponent::ClearAllTargets()
{
    Target.Target.ComponentProperty = NAME_None;
    Target.BoneName.Empty();
    Target.Target.OtherActor = nullptr;
}

void UFollowComponent::SetFollow(FBlackEyeSimpleTarget NewTarget, bool Force)
{
    if (!Force && NewTarget == this->Target)
    {
        return;
    }

    if (NewTarget.IsValid())
    {
        this->SnapFollow = true;
    }
    else
    {
        DesiredRotation = this->GetComponentQuat();
    }

    this->Target = NewTarget;


#if WITH_EDITOR
    this->LastSeenFollow = NewTarget;
#endif
}

void UFollowComponent::SetFollow(USceneComponent* Component, FString BoneName)
{
    FBlackEyeSimpleTarget NewTarget;
    NewTarget.Target.ComponentProperty = NAME_None;
    NewTarget.BoneName = BoneName;
    if (Component != nullptr)
    {
        NewTarget.Target.OtherActor = Component->GetOwner();
        NewTarget.Target.PathToComponent = Component->GetPathName(Component->GetOwner());
    }

    this->Target = NewTarget;

    if (this->Target.IsValid())
    {
        this->SnapFollow = true;
    }
    else
    {
        DesiredRotation = this->GetComponentQuat();
    }

#if WITH_EDITOR
    // Copy changes to last seen
    this->LastSeenFollow = this->Target;
#endif
}

void UFollowComponent::SetFollowActorOverride(AActor* Actor)
{
    this->Target.Target.OtherActor = Actor;
    if (this->Target.IsValid())
    {
        this->SnapFollow = true;
    }
    else
    {
        DesiredRotation = this->GetComponentQuat();
    }

#if WITH_EDITOR
    // Copy changes to last seen
    this->LastSeenFollow = this->Target;
#endif
}

#if WITH_EDITOR
void UFollowComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{

    this->YawOffset = FBlackEyeMath::AngleRepeat(this->YawOffset);
    this->PitchOffset = FBlackEyeMath::AngleRepeat(this->PitchOffset);
    this->RollOffset = FBlackEyeMath::AngleRepeat(this->RollOffset);

    if (this->Targets.Num() == 0)
    {
        this->Targets.Add(this->Target);
    }
    else
    {
        if (this->bUseMultipleTargets)
        {
            this->Target = this->Targets[0];
        }
        else
        {
            this->Targets[0] = this->Target;
        }
    }

    if ((PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UFollowComponent, bUseMultipleTargets))
        || (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UFollowComponent, OrientationReferenceMode)))
    {
        FTransform NewTransform;
        FVector NewFollowPoint;
        if (GetTargetTransform(this->OrientationReferenceMode, this->bUseMultipleTargets, NewTransform, NewFollowPoint))
        {
            FVector NewFollowOffset = NewTransform.InverseTransformPosition(this->GetComponentLocation());

            this->FollowOffset = NewFollowOffset;
            this->TargetReferenceToWorld = NewTransform;
        }
        else
        {
            this->FollowOffset = FVector::ZeroVector;
            this->TargetReferenceToWorld = this->GetComponentTransform();
        }

        //TArray<USceneComponent*> ChildComponents;
        //this->GetChildrenComponents(true, ChildComponents);
        //for (const USceneComponent* Child : ChildComponents)
        //{
        //    if (Child->Implements<UBlackEyeHasFollow>())
        //    {
        //        ((IBlackEyeHasLookAt*)Child)->SnapToTargets();
        //    }
        //}
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UFollowComponent::ProcessActorEditorTranslationDelta(const FVector& Delta)
{
    if (HasValidTargets)
    {
        this->FollowOffset += this->TargetReferenceToWorld.InverseTransformVector(Delta);
        MarkPackageDirty();
    }
}
#endif


