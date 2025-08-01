// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "Components/LookAtComponent.h"

#include "BlackEyeCVars.h"

#include "Utility/BlackEyeMath.h"
#include "Utility/BlackEyeUtilities.h"

#include "Engine/World.h"

#include "GameFramework/PlayerController.h"

#include "Runtime/Engine/Public/UnrealClient.h"

#include "Runtime/CinematicCamera/Public/CineCameraComponent.h"

#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"

#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#include "Modules/ModuleManager.h"
#endif

// Turn human readable 0-100% value into a [0,1] range
#define VIEWPORT_SIZE_SCALE_FACTOR 0.01f
#define LOOK_AT_SNAP_SETTLE_FRAME_COUNT 2

DECLARE_CYCLE_STAT(TEXT("Look At Component Tick"), STAT_LookAtComponent, STATGROUP_Game);

FVector2D ULookAtComponent::GetLookAtViewportPosition(EViewLookPosition Position)
{
    // Values are based on (0,0) being bottom left of the viewport
    switch (Position)
    {
    case EViewLookPosition::VLP_TopLeft:
        return FVector2D(1.f / 3.f, 2.f / 3.f);

    case EViewLookPosition::VLP_TopRight:
        return FVector2D(2.f / 3.f, 2.f / 3.f);

    case EViewLookPosition::VLP_BottomLeft:
        return FVector2D(1.f / 3.f, 1.f / 3.f);

    case EViewLookPosition::VLP_BottomRight:
        return FVector2D(2.f / 3.f, 1.f / 3.f);

    case EViewLookPosition::VLP_Center:
    default:
        return FVector2D(0.5f, 0.5f);
    }
}

ULookAtComponent::ULookAtComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostPhysics;

    bRollRelativeToParent = true;
    Roll = 0.f;

    TargetCombineMode = EViewTargeCombineModes::VTCM_AllTargets;
    TargetResolveType = EViewTargetResolveModes::VTRM_ScreenCenter;

    Damping.bLinkDamping = true;
    Damping.YawDamping = 0.3f;
    Damping.PitchDamping = 0.3f;

    LookAheadVelocityVector = FVector::ZeroVector;
    VelocityLookAheadTime = 0.f;
    LookAheadVelocityVelocity = 0.f;

    bKeepTargetOnScreen = false;
    TargetOffset = FVector::ZeroVector;
    ScreenPosition = GetLookAtViewportPosition(EViewLookPosition::VLP_Center);
    LastRotationLocal = FQuat::Identity;

    bDynamicFoV = false;
    MaxFoV = 100.f;
    MinFoV = 12.f;
    FieldOfViewDamping = 0.1f;
    FieldOfViewVelocity = 0.f;
    DesiredTargetViewportSize = 50.f;

    bSetFocalDistance = true;

    SnapSettleFrameCounter = 0;
}

void ULookAtComponent::BeginPlay()
{
    Super::BeginPlay();

    this->RootActorComponent = nullptr;
    this->bUseVelocityLookAhead = true;
    CacheRootCamActorComponent();
}

void ULookAtComponent::CacheRootCamActorComponent()
{
    if (!this->RootActorComponent.IsValid())
    {
        this->RootActorComponent = Cast<ABlackEyeCameraRigBase>(this->GetOwner());
    }
}

bool ULookAtComponent::CameraExistsInActor() const
{
    if (this->RootActorComponent.IsValid())
    {
        return this->RootActorComponent->GetCamera().IsValid();
    }

    return false;
}

// Called every frame
void ULookAtComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    SCOPE_CYCLE_COUNTER(STAT_LookAtComponent);

#if WITH_EDITOR
    CacheRootCamActorComponent();
#endif

    this->SnapRotation |= FBlackEyeCVars::DampingDisabled();

    if (CameraExistsInActor())
    {
        UCameraComponent* CamComponent = this->RootActorComponent->GetCamera().Get();
        UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(CamComponent);
        const USceneComponent* ParentComponent = this->GetAttachParent();
        FRotator MountParentRotation = FRotator();
        if (ParentComponent != nullptr)
        {
            MountParentRotation = ParentComponent->GetComponentRotation();
        }

        FVector MountWorldPosition = this->GetComponentLocation();
        FTransform WorldToMountTransform = FTransform(MountParentRotation.Quaternion(), MountWorldPosition);

        float ViewportAspectRatio = 16.f / 9.f;
        FIntPoint ViewportSize;
#if WITH_EDITOR
        // There's an implicit assumption in GEditor->GetActiveViewport() that the level editor module exists and is loaded. 
        // In MRQ remote renderers, the level editor module is not leaded. Early out here for MRQ issues
        if (!GetWorld()->IsPlayInEditor() && FModuleManager::Get().IsModuleLoaded(TEXT("LevelEditor")))
        {
            const FViewport* ActiveView = GEditor->GetActiveViewport();
            if (!ActiveView) return;

            if (CineCamera != nullptr)
            {
                ViewportAspectRatio = CineCamera->Filmback.SensorAspectRatio;
            }
            else
            {
                ViewportAspectRatio = CamComponent->AspectRatio;
            }

            ViewportSize = ActiveView->GetSizeXY();
        }
        else
#endif
        {
            const FViewport* ActiveView = GetLocalViewport();
            if (!ActiveView) return;

            if (CineCamera != nullptr)
            {
                ViewportAspectRatio = CineCamera->Filmback.SensorAspectRatio;
            }
            else
            {
                ViewportAspectRatio = CamComponent->AspectRatio;
            }

            ViewportSize = ActiveView->GetSizeXY();
        }

        ViewportAspectRatio = CamComponent->bConstrainAspectRatio ? ViewportAspectRatio : ((float)ViewportSize.X / (float)ViewportSize.Y);

        FVector CalculatedLookAtVectorWorld;
        FRotator PreviousMountRotation = WorldToMountTransform.TransformRotation(this->LastRotationLocal).Rotator();
        FRotator MountRotationWorld = this->GetComponentRotation();

        FBox TargetWorldBox;
        FBox2D TargetViewportBox;

        // HACK: I hate where this is but we can't put lookahead damping calculation inside of GetLookAtMetrics()
        if (bUseVelocityLookAhead
            && TargetCombineMode == EViewTargeCombineModes::VTCM_First
            && TargetResolveType == EViewTargetResolveModes::VTRM_WorldCenter
            && LookAtTargets.Num() == 1 && LookAtTargets[0].IsValid())
        {
            FVector CurrVelocity = LookAtTargets[0].GetTargetVelocity();
            this->LookAheadVelocityVector = FBlackEyeMath::SmoothDamp(LookAheadVelocityVector,
                CurrVelocity,
                this->LookAheadVelocityVelocity,
                this->VelocityLookAheadDamping,
                DeltaTime);
        }

        FVector CurrentLensLocation = CamComponent->GetComponentLocation();
        bool HasValidTarget = GetLookAtMetrics(CurrentLensLocation, MountRotationWorld, ViewportSize, ViewportAspectRatio, CalculatedLookAtVectorWorld, TargetWorldBox, TargetViewportBox);

        if (!HasValidTarget) return;

        // Use 1cm as this limit for now (0.1^2 == 0.01)
        const float k_MinDistanceForLookAt = 0.1f;

        // Only evalute our look when there is sufficient distance between us and our look at target otherwise the math breaks down. 
        if (CalculatedLookAtVectorWorld.SquaredLength() > k_MinDistanceForLookAt * k_MinDistanceForLookAt)
        {
            FBox2D ViewportTargetBox = GetTargetViewportBox();
            FVector2D CurrentTargetViewportPosition;

            // Use current camera position so that the frustum/viewport are from the correct location
            FVector CameraComponentWorldPosition = CamComponent->GetComponentLocation();
            FRotator CameraComponentRotator = CamComponent->GetComponentRotation();
            FRotator CameraMountArmRotator = CamComponent->GetRelativeRotation();

            FSceneViewProjectionData ProjectionData = FBlackEyeUtilities::GetViewProjectionData(CamComponent, CameraComponentWorldPosition, CameraComponentRotator, ViewportSize, ViewportAspectRatio);

            // Look at vector will either be the delta to our target (world space), or the vector to the target screen position. Either way, 
            // use the resultant vector to project our cam position to make a world point
            FVector WorldLookAtPoint = CameraComponentWorldPosition + CalculatedLookAtVectorWorld;
            CurrentTargetViewportPosition = FBlackEyeUtilities::GetViewportPoint(ProjectionData, WorldLookAtPoint);

            if (!ViewportTargetBox.IsInside(CurrentTargetViewportPosition) || SnapRotation)
            {
                FVector2D ClosestPointToDeadZone = ViewportTargetBox.GetClosestPointTo(CurrentTargetViewportPosition);
                FVector LookVectorToDeadZoneEdge = FBlackEyeUtilities::GetWorldVectorFromViewportPoint(ProjectionData, ClosestPointToDeadZone);

                // Calculate interpolated rotation for object (centered) before adding our look at screen offset
                FRotator RotatorToEdgeOfDeadZone = LookVectorToDeadZoneEdge.Rotation();
                FRotator DeltaToEdgeOfDeadZone = RotatorToEdgeOfDeadZone - PreviousMountRotation;
                FRotator DesiredRotator = CalculatedLookAtVectorWorld.ToOrientationRotator();

                FRotator FinalRotation;
                if (this->SnapRotation)
                {
                    FinalRotation = DesiredRotator;
                }
                else
                {
                    float PitchDamping = this->Damping.bLinkDamping ? this->Damping.YawDamping : this->Damping.PitchDamping;

                    float PitchEuler = FBlackEyeMath::SmoothDampAngle(PreviousMountRotation.Pitch + DeltaToEdgeOfDeadZone.Pitch,
                        DesiredRotator.Pitch,
                        this->PitchVelocity,
                        PitchDamping,
                        DeltaTime);
                    float YawEuler = FBlackEyeMath::SmoothDampAngle(PreviousMountRotation.Yaw + DeltaToEdgeOfDeadZone.Yaw,
                        DesiredRotator.Yaw,
                        this->YawVelocity,
                        this->Damping.YawDamping,
                        DeltaTime);

                    FinalRotation = FRotator(PitchEuler, YawEuler, 0.) - DeltaToEdgeOfDeadZone - CameraMountArmRotator;
                }

                // Always set roll after the fact
                FinalRotation.Roll = Roll;

                const USceneComponent* Parent = GetAttachParent();
                if (this->bRollRelativeToParent && Parent)
                {
                    FinalRotation.Roll += Parent->GetComponentTransform().Rotator().Roll;
                }

                // Store the rotation with the target centered, before we apply an offset for now.
                LastRotationLocal = WorldToMountTransform.InverseTransformRotation(FinalRotation.Quaternion());

#if WITH_EDITOR
                // Apply our screen space look at offset to our desired screen position. For debug purposes
                FSceneViewProjectionData UpdatedProjectionData = FBlackEyeUtilities::GetViewProjectionData(CamComponent, CameraComponentWorldPosition, FinalRotation, ViewportSize, ViewportAspectRatio);
                FVector WorldDirection = FBlackEyeUtilities::GetWorldVectorFromViewportPoint(UpdatedProjectionData, this->ScreenPosition);
                LastLookAtVectorWorld = ViewportTargetBox.IsInside(CurrentTargetViewportPosition) ? WorldDirection : LookVectorToDeadZoneEdge;
                // Store our desired/actual look ats for debugging
                DesiredLookAtVectorWorld = CalculatedLookAtVectorWorld;
#endif
                this->SetRelativeRotation(LastRotationLocal);

                //Ensure there is no offset/rotation to the sub-camera relative to us
                CamComponent->SetRelativeLocation(FVector::ForwardVector * CameraPlateDistance + FVector::UpVector * CameraPedestalHeight);
                CamComponent->SetRelativeRotation(FQuat::Identity);
            }
            else
            {
                PitchVelocity = 0.f;
                YawVelocity = 0.f;
            }

            // After we've decided on a new look at rotation, decide how to adjust FoV, force target on screen, etc
            if (this->bDynamicFoV || this->bSetFocalDistance || this->bKeepTargetOnScreen)
            {
                FBox2D TargetBox;
                FBox WorldBox;

                FRotator CurrentRotationWorld = WorldToMountTransform.TransformRotation(LastRotationLocal).Rotator();
                CameraComponentWorldPosition = CamComponent->GetComponentLocation();
                CameraComponentRotator = CamComponent->GetComponentRotation();

                //Build dynamic FoV from the projection data, using the rotation where our target was centered on screen
                ProjectionData = FBlackEyeUtilities::GetViewProjectionData(CamComponent, CameraComponentWorldPosition, CameraComponentRotator, ViewportSize, ViewportAspectRatio);
                GetTargetGroupViewportBoundingBox(ProjectionData, TargetBox, WorldBox);

                bool HasFoVChanged = false;

                if (this->bDynamicFoV)
                {
                    FVector2D ApparentSize = TargetBox.GetSize();

                    // HACK: Not sure why this is such an issue here but the view projection data not matching what we calculate inside of the component vs visualizeres.
                    // But only when playing in editor.
                    float AspectRatioRatio = GetWorld()->IsPlayInEditor() ? ViewportAspectRatio / CamComponent->AspectRatio : 1.f;
                    ApparentSize /= AspectRatioRatio;

                    double MaxSize = FMath::Max(ApparentSize.X, ApparentSize.Y);
                    double ApparentScaleDelta = MaxSize / (DesiredTargetViewportSize * VIEWPORT_SIZE_SCALE_FACTOR);

                    double CurrentFoV = (CineCamera != nullptr) ? CineCamera->GetHorizontalFieldOfView() : CamComponent->FieldOfView;
                    double CurrentFoVHalfAngle = CurrentFoV * 0.5f;

                    double OppositeSideLength = FMath::Tan(FMath::DegreesToRadians(CurrentFoVHalfAngle));

                    // Scale the frustum opposite length by the % we need the target box to grow or shrink. Let the Clamp() below sort out singularities
                    double AdjustedOppositeSideLength = OppositeSideLength * ApparentScaleDelta;
                    double CalculatedFoV = FMath::RadiansToDegrees(FMath::Atan(AdjustedOppositeSideLength)) * 2.;
                    RequestedFoV = CalculatedFoV;

                    CalculatedFoV = FMath::Clamp(CalculatedFoV, this->MinFoV, this->MaxFoV);
                    double FinalFoV = FBlackEyeMath::SmoothDamp(CurrentFoV, CalculatedFoV, FieldOfViewVelocity, this->FieldOfViewDamping, DeltaTime);

                    HasFoVChanged = !FMath::IsNearlyEqual(FinalFoV, CalculatedFoV, UE_KINDA_SMALL_NUMBER);

                    CamComponent->SetFieldOfView(FinalFoV);
                }

                if (this->bKeepTargetOnScreen)
                {
                    // Recalculate the projection params and boxes based on new FoV
                    if (HasFoVChanged)
                    {
                        CameraComponentWorldPosition = CamComponent->GetComponentLocation();
                        CameraComponentRotator = CamComponent->GetComponentRotation();
                        ProjectionData = FBlackEyeUtilities::GetViewProjectionData(CamComponent, CameraComponentWorldPosition, CameraComponentRotator, ViewportSize, ViewportAspectRatio);
                        GetTargetGroupViewportBoundingBox(ProjectionData, TargetBox, WorldBox);
                    }

                    FVector2D TargetBoxSize = TargetBox.GetSize();

                    bool OverScannedYaw = TargetBoxSize.X > 1.f;
                    bool OverScannedPitch = TargetBoxSize.Y > 1.f;

                    FVector2D TargetBoxMin = TargetBox.Min;
                    FVector2D TargetBoxMax = TargetBox.Max;
                    const FBox2D ViewportBox(FVector2D::ZeroVector, FVector2D::One());

                    bool MinIsOutside = ViewportBox.ComputeSquaredDistanceToPoint(TargetBoxMin) > 0.f;
                    bool MaxIsOutside = ViewportBox.ComputeSquaredDistanceToPoint(TargetBoxMax) > 0.f;

                    bool AnyScreenPointsOffScreen = MinIsOutside || MaxIsOutside;
                    FRotator RotationDeltaToSnapTargetOnScreen = FRotator::ZeroRotator;
                    if (AnyScreenPointsOffScreen)
                    {
                        // Check pitch/yaw adjustments independently
                        if (!OverScannedYaw && (TargetBoxMin.X < 0.f || TargetBoxMax.X > 1.f))
                        {
                            FVector2D ViewportPoint(TargetBoxMin.X < 0.f ? 0.f : 1.f, 0.5f);
                            FVector2D OffScreenReferencePt(TargetBoxMin.X < 0.f ? TargetBoxMin.X : TargetBoxMax.X, 0.5f);
                            FVector MinPointViewRay = FBlackEyeUtilities::GetWorldVectorFromViewportPoint(ProjectionData, OffScreenReferencePt);
                            FVector ClosestPointOnViewport = FBlackEyeUtilities::GetWorldVectorFromViewportPoint(ProjectionData, ViewportPoint);

                            RotationDeltaToSnapTargetOnScreen = (MinPointViewRay.Rotation() - ClosestPointOnViewport.Rotation());

                            this->YawVelocity = 0.f;
                        }

                        if (!OverScannedPitch && (TargetBoxMin.Y < 0.f || TargetBoxMax.Y > 1.f))
                        {
                            FVector2D ViewportPoint(0.5f, TargetBoxMin.Y < 0.f ? 0.f : 1.f);
                            FVector2D OffScreenReferencePt(0.5f, TargetBoxMin.Y < 0.f ? TargetBoxMin.Y : TargetBoxMax.Y);
                            FVector MinPointViewRay = FBlackEyeUtilities::GetWorldVectorFromViewportPoint(ProjectionData, OffScreenReferencePt);
                            FVector ClosestPointOnViewport = FBlackEyeUtilities::GetWorldVectorFromViewportPoint(ProjectionData, ViewportPoint);

                            RotationDeltaToSnapTargetOnScreen += (MinPointViewRay.Rotation() - ClosestPointOnViewport.Rotation());

                            this->PitchVelocity = 0.f;
                        }

                        // Ramp down the keep on screen force as we approach overscan in both axes
                        float KeepOnScreenStrength = 1.f - FBlackEyeMath::InverseLerp(FMath::Min(TargetBoxSize.X, TargetBoxSize.Y), 0.75f, 1.f);
                        RotationDeltaToSnapTargetOnScreen *= FMath::SmoothStep(0.f, 1.f, KeepOnScreenStrength);

                        // Correct our final rotations, and set our updated world rotation
                        FRotator FinalRotation = (CurrentRotationWorld + RotationDeltaToSnapTargetOnScreen);
                        LastRotationLocal = WorldToMountTransform.InverseTransformRotation(FinalRotation.Quaternion());

                        this->SetRelativeRotation(LastRotationLocal);
                    }
                }

                if (bSetFocalDistance)
                {
                    float FocalDistance = (WorldBox.GetCenter() - MountWorldPosition).Length() + this->FocalDistanceOffset;
                    if (CineCamera != nullptr)
                    {
                        CineCamera->FocusSettings.ManualFocusDistance = FocalDistance;
                    }
                    else
                    {
                        CamComponent->PostProcessSettings.DepthOfFieldFocalDistance = FocalDistance;
                    }
                }
            }
        }
#if WITH_EDITOR
        else
        {
            this->DesiredLookAtVectorWorld = FVector::Zero();
        }
#endif

        if (SnapRotation)
        {
#if WITH_EDITOR
            //if (GUnrealEd && GUnrealEd->PlayWorld)
           //      GUnrealEd->PlayWorld->bDebugPauseExecution = true;
#endif

            // Hack: the fact that this is not perfect the first time around points to a bug we're covering up or an unknown order of operations on snap.
            if (SnapSettleFrameCounter == LOOK_AT_SNAP_SETTLE_FRAME_COUNT)
            {
                this->SnapRotation = false;
                SnapSettleFrameCounter = 0;
            }
            else
            {
                SnapSettleFrameCounter++;
            }

            PitchVelocity = 0.f;
            YawVelocity = 0.f;
            FieldOfViewVelocity = 0.f;

            LookAheadVelocityVector = FVector::ZeroVector;
            LookAheadVelocityVelocity = 0.f;
        }
    }
}

#if WITH_EDITOR
void ULookAtComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    this->Roll = FBlackEyeMath::AngleRepeat(this->Roll);

    this->MinFoV = FMath::Clamp(MinFoV, 0., MaxFoV);
    this->MaxFoV = FMath::Clamp(MaxFoV, MinFoV + SMALL_NUMBER, 180.);

    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

int ULookAtComponent::GetNumViewTargets() const
{
    return this->LookAtTargets.Num();
}

void ULookAtComponent::SetViewTargetCombineMode(EViewTargeCombineModes ResolveMode)
{
    this->TargetCombineMode = ResolveMode;
}

void ULookAtComponent::SnapToTargets()
{
    this->SnapRotation = true;
}

bool ULookAtComponent::GetTargetAtIndex(FBlackEyeTarget& Target, int Index) const
{
    if (Index < this->LookAtTargets.Num())
    {
        Target = this->LookAtTargets[Index];
        return true;
    }

    return false;
}

void ULookAtComponent::RemoveTargetAtIndex(int Index)
{
    if (Index < this->LookAtTargets.Num())
    {
        this->LookAtTargets.RemoveAt(Index);
    }
}

void ULookAtComponent::ClearAllTargets()
{
    this->LookAtTargets.Reset();
}

int ULookAtComponent::GetNumTargets()
{
    return this->LookAtTargets.Num();
}

int ULookAtComponent::AddLookAt(FBlackEyeTarget Target, bool RequestSnap)
{
    this->LookAtTargets.Add(Target);
    this->SnapRotation |= RequestSnap;

    return this->LookAtTargets.Num() - 1;
}

int ULookAtComponent::AddLookAt(class USceneComponent* Component, FString BoneName, bool UseAutoSize, float BoundingRadius, bool RequestSnap)
{
    FBlackEyeTarget NewTarget;
    if (Component != nullptr)
    {
        NewTarget.Target.OtherActor = Component->GetOwner();
        NewTarget.Target.PathToComponent = Component->GetPathName(Component->GetOwner());
    }
    else
    {
        NewTarget.Target.OtherActor = nullptr;
    }

    NewTarget.Target.ComponentProperty = NAME_None;
    NewTarget.BoneName = BoneName;
    NewTarget.bAutoSize = UseAutoSize;
    NewTarget.BoundingRadius = BoundingRadius;

    this->LookAtTargets.Add(NewTarget);

    this->SnapRotation |= RequestSnap;

    return this->LookAtTargets.Num() - 1;
}

bool ULookAtComponent::RemoveTarget(int Index, bool RequestSnap)
{
    bool WillSucceed = Index < this->LookAtTargets.Num();
    this->LookAtTargets.RemoveAt(Index);

    this->SnapRotation |= RequestSnap;

    return WillSucceed;
}

void ULookAtComponent::SetLookAt(FBlackEyeTarget Target, int Index, bool RequestSnap)
{
    if (Index < this->LookAtTargets.Num())
    {
        this->LookAtTargets[Index] = Target;
    }
    else if (Index == this->LookAtTargets.Num())
    {
        this->LookAtTargets.Add(Target);
    }

    this->SnapRotation |= RequestSnap;
}

void ULookAtComponent::SetLookAt(USceneComponent* Component, FString BoneName, int Index, bool UseAutoSize, float BoundingRadius, bool RequestSnap)
{
    FBlackEyeTarget NewTarget;
    if (Component != nullptr)
    {
        NewTarget.Target.OtherActor = Component->GetOwner();
        NewTarget.Target.PathToComponent = Component->GetPathName(Component->GetOwner());
    }
    else
    {
        NewTarget.Target.OtherActor = nullptr;
    }

    NewTarget.Target.ComponentProperty = NAME_None;
    NewTarget.BoneName = BoneName;
    NewTarget.bAutoSize = UseAutoSize;
    NewTarget.BoundingRadius = BoundingRadius;

    SetLookAt(NewTarget, Index, RequestSnap);
}

void ULookAtComponent::SetLookAtActorOverride(AActor* TargetActor, int Index, bool RequestSnap)
{
    SetViewTargetCombineMode(EViewTargeCombineModes::VTCM_First);
    if (this->LookAtTargets.Num() > 0)
    {
        this->LookAtTargets[0].Target.OtherActor = TargetActor;
    }
    else
    {
        FBlackEyeTarget NewTarget;
        NewTarget.Target.OtherActor = TargetActor;
        USceneComponent* rootComponent = TargetActor->GetRootComponent();
        NewTarget.Target.ComponentProperty = NAME_None;
        NewTarget.Target.PathToComponent = rootComponent->GetPathName(TargetActor);
        NewTarget.bAutoSize = true;
        this->LookAtTargets.Add(NewTarget);
    }

    this->SnapRotation |= RequestSnap;
}

bool ULookAtComponent::GetLookAtMetrics(FVector FromPoint,
    const FRotator& WithRotation,
    FIntPoint ViewportSize,
    float ViewportAspectRatio,
    FVector& /* out */ OutResolvedLookVector,
    FBox& /* out */ OutWorldBox,
    FBox2D& /* out */ OutScreenSpaceBox) const
{
    UCameraComponent* CamComponent = this->RootActorComponent->GetCamera().Get();
    FSceneViewProjectionData ProjectionData = FBlackEyeUtilities::GetViewProjectionData(CamComponent, FromPoint, WithRotation, ViewportSize, ViewportAspectRatio);
    bool HasValidTarget = GetTargetGroupViewportBoundingBox(ProjectionData, OutScreenSpaceBox, OutWorldBox);

    switch (this->TargetResolveType)
    {
    case EViewTargetResolveModes::VTRM_WorldCenter:
    {
        if (HasValidTarget)
        {
            if (this->TargetCombineMode == EViewTargeCombineModes::VTCM_First)
            {
                FVector Position;
                FQuat Rotation;
                FBlackEyeTarget FirstTarget = this->LookAtTargets[0];
                FirstTarget.GetTargetPositionAndRotation(Position, Rotation, this->bOffsetInTargetLocalSpace ? this->TargetOffset : FVector::ZeroVector);
                if (!this->bOffsetInTargetLocalSpace)
                {
                    Position += this->TargetOffset;
                }

                //TODO: Better way to not try to calculate this at edit time
                if (this->bUseVelocityLookAhead)
                {
                    Position += this->LookAheadVelocityVector * this->VelocityLookAheadTime;
                }

                OutResolvedLookVector = Position - FromPoint;
            }
            else
            {
                OutResolvedLookVector = (OutWorldBox.GetCenter() + this->TargetOffset) - FromPoint;
            }
        }
    }
    break;
    case EViewTargetResolveModes::VTRM_ScreenCenter:
    {
        if (HasValidTarget)
        {
            FVector WorldLookAtPoint = OutWorldBox.GetCenter();

            FVector VectorToWorldBoxCenter = WorldLookAtPoint - FromPoint;
            VectorToWorldBoxCenter.Normalize();

            FVector2D BoxSize = OutScreenSpaceBox.GetSize();
            float MaxSize = FMath::Max(BoxSize.X, BoxSize.Y);

            FVector VectorToScreenBoxCenter = FBlackEyeUtilities::GetWorldVectorFromViewportPoint(ProjectionData, OutScreenSpaceBox.GetCenter());
            float DotProductForwardAndVectorToTarget = VectorToWorldBoxCenter.Dot(VectorToScreenBoxCenter);

            // From 45 to 60 degrees of misalignment between screen and world vectors, start looking at the world center
            float AdditionalTValueFromMisalignedLookAt = FBlackEyeMath::InverseLerp(DotProductForwardAndVectorToTarget, 0.707f, 0.5f);

            //Arbitrary "we are getting close to the world bounds" hand off to gracefully transition as we enter the bounds
            float AdditionalTFromWorldBoxProximity = FBlackEyeMath::InverseLerp(OutWorldBox.ComputeSquaredDistanceToPoint(FromPoint), 10.f, 0.f);

            // Hand over to world box center if our box becomes too large (can cause instabilities)
            float AdditionalTFromScreenBoxOversize = FBlackEyeMath::InverseLerp(MaxSize, 1.25f, 2.5f);
            float tToWorldCenter = FMath::Clamp(AdditionalTFromScreenBoxOversize + AdditionalTValueFromMisalignedLookAt + AdditionalTFromWorldBoxProximity, 0.f, 1.f);

            tToWorldCenter = FMath::SmoothStep(0.f, 1.f, tToWorldCenter);
#if ENGINE_MINOR_VERSION > 1
            OutResolvedLookVector = FVector::SlerpNormals(VectorToScreenBoxCenter, VectorToWorldBoxCenter, tToWorldCenter);
#else
            OutResolvedLookVector = FBlackEyeUtilities::SlerpNormals(VectorToScreenBoxCenter, VectorToWorldBoxCenter, tToWorldCenter);
#endif
            //if (tToWorldCenter > 0.f)
            //{
            //    UE_LOG(LogTemp, Display, TEXT("(%s) Misalignment: %.3f. Oversize: %.3f. Proximity: %.3f. Final T-value: %.3f"),
            //                                    *GetOwner()->GetActorNameOrLabel(),
            //                                    additionalTValueFromMisalignedLookAt,
            //                                    additionalTFromScreenBoxOversize,
            //                                    additionalTFromWorldBoxProximity,
            //                                    tToWorldCenter);
            //}
        }
    }
    break;
    }

    return HasValidTarget;
}

bool ULookAtComponent::GetTargetGroupBoundingVolume(FBox& OutTargetBox) const
{
    int NumValidTargets = 0;
    TArray<FBlackEyeTarget> Targets = this->LookAtTargets;

    for (int i = 0; i < Targets.Num(); ++i)
    {
        if (i > 0 && this->TargetCombineMode == EViewTargeCombineModes::VTCM_First) break;

        if (Targets[i].IsValid())
        {
            if (NumValidTargets == 0)
            {
                Targets[i].GetTargetBounds(OutTargetBox);
            }
            else
            {
                FBox nextBox;
                Targets[i].GetTargetBounds(nextBox);

                OutTargetBox += nextBox;
            }
            NumValidTargets++;
        }
    }

    return NumValidTargets > 0;
}

bool ULookAtComponent::GetTargetGroupViewportBoundingBox(const FSceneViewProjectionData& ProjectionData, FBox2D& OutTargetBox, FBox& OutWorldBox) const
{
    TArray<FBlackEyeTarget> Targets = this->LookAtTargets;
    int NumValidTargets = 0;

    FVector Vertices[8];
    FBox TempWorldBox;

    FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
    for (int i = 0; i < Targets.Num(); ++i)
    {
        if (i > 0 && this->TargetCombineMode == EViewTargeCombineModes::VTCM_First) break;

        const FBlackEyeTarget& Target = Targets[i];
        if (Target.IsValid())
        {
            if (NumValidTargets == 0)
            {
                Targets[i].GetTargetBounds(TempWorldBox);
                OutWorldBox = TempWorldBox;
                TempWorldBox.GetVertices(Vertices);

                FVector2D ViewportPoint = FBlackEyeUtilities::GetViewportPoint(ViewProjectionMatrix, Vertices[0]);
                OutTargetBox = FBox2D(ViewportPoint, ViewportPoint);

                for (int j = 1; j < 8; ++j)
                {
                    ViewportPoint = FBlackEyeUtilities::GetViewportPoint(ViewProjectionMatrix, Vertices[j]);
                    OutTargetBox += ViewportPoint;
                }
            }
            else
            {
                Targets[i].GetTargetBounds(TempWorldBox);
                OutWorldBox += TempWorldBox;
                TempWorldBox.GetVertices(Vertices);
                for (int j = 0; j < 8; ++j)
                {
                    FVector2D viewportPoint = FBlackEyeUtilities::GetViewportPoint(ViewProjectionMatrix, Vertices[j]);
                    OutTargetBox += viewportPoint;
                }
            }

            NumValidTargets++;
        }
    }

    return NumValidTargets > 0;
}

const FViewport* ULookAtComponent::GetLocalViewport() const
{
    //TODO: We need to be fed this from the player controller or manager
    APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    ULocalPlayer* const LP = PC ? PC->GetLocalPlayer() : nullptr;

    return LP ? LP->ViewportClient->Viewport : nullptr;
}

FBox2D ULookAtComponent::GetTargetViewportBox() const
{
    FBox2D ViewportBox(this->ScreenPosition - this->ScreenDeadZoneSize * 0.5f, this->ScreenPosition + this->ScreenDeadZoneSize * 0.5f);

    ViewportBox.Min = FVector2D(FMath::Clamp(ViewportBox.Min.X, 0., 1.), FMath::Clamp(ViewportBox.Min.Y, 0., 1.));
    ViewportBox.Max = FVector2D(FMath::Clamp(ViewportBox.Max.X, 0., 1.), FMath::Clamp(ViewportBox.Max.Y, 0., 1.));
    return ViewportBox;
}
