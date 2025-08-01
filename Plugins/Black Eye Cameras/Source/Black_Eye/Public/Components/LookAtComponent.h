// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "Actors/BlackEyeCameraRigBase.h"

#include "Interfaces/BlackEyeHasLookAt.h"

#include "Utility/BlackEyeCompositionDamping.h"

#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

#include "LookAtComponent.generated.h"

UENUM(BlueprintType)
enum class EViewLookPosition : uint8
{
    VLP_Center          UMETA(DisplayName = "Center"),
    VLP_TopLeft         UMETA(DisplayName = "Top Left"),
    VLP_TopRight        UMETA(DisplayName = "Top Right"),
    VLP_BottomLeft      UMETA(DisplayName = "Bottom Left"),
    VLP_BottomRight     UMETA(DisplayName = "Bottom Right")
};

UENUM(BlueprintType)
enum EViewTargeCombineModes
#if ENGINE_MINOR_VERSION > 2
    : uint8
#endif
{
    VTCM_First                  UMETA(DisplayName = "First Subject Only"),
    VTCM_AllTargets             UMETA(DisplayName = "All Subjects")
};

UENUM(BlueprintType)
enum EViewTargetResolveModes
#if ENGINE_MINOR_VERSION > 2
    : uint8
#endif
{
    VTRM_ScreenCenter           UMETA(DisplayName = "Screenspace Center", Tooltip = "Screenspace center tracks the subject’s projected 3D volume into 2D screenspace"),
    VTRM_WorldCenter            UMETA(DisplayName = "Worldspace Center", Tooltip = "Worldspace center tracks the center point of the subject projected onto the screen.")
};

/**
 * A component which updates its rotation in order to maintain composition of one or more subjects in a child camera
 */
UCLASS(ClassGroup = "BlackEye", hideCategories = (Input, Rendering), meta = (BlueprintSpawnableComponent))
class BLACK_EYE_API ULookAtComponent : public USceneComponent, public IBlackEyeHasLookAt
{
    GENERATED_BODY()

public:
    ULookAtComponent();

    UPROPERTY(EditAnywhere, Interp, Category = "Look", meta = (Tooltip = "Whether or not the Roll value specified is an additional offset to the Roll from the parent component, or a world space roll. Imagine an action camera with a fixed mount a vehicle. Relative to parent would allow it to roll with the vehicle or have a fixed horizon."))
    bool bRollRelativeToParent;

    UPROPERTY(EditAnywhere, Interp, Category = "Look", meta = (Tooltip = "Specifies how much roll (dutch) the camera adds when it looks at its subject(s).", Units = "deg"))
    float Roll;

    UPROPERTY(EditAnywhere, Category = "Look", meta = (Tooltip = "Moves the camera component along the forward axis to offset the camera's pivot point."))
    float CameraPlateDistance;

    UPROPERTY(EditAnywhere, Category = "Look", meta = (Tooltip = "Moves the camera component along the up axis to offset the camera's pivot point."))
    float CameraPedestalHeight;

    UPROPERTY(EditAnywhere, Category = "Look", DisplayName = "Subject(s)")
    TArray<FBlackEyeTarget> LookAtTargets;

    UPROPERTY(EditAnywhere, Category = "Look", DisplayName = "Subject Combine Mode", meta = (Tooltip = "Defines how we combine all subject(s) in our subject list when processing look at calculations."))
    TEnumAsByte<EViewTargeCombineModes> TargetCombineMode;

    UPROPERTY(EditAnywhere, Category = "Look", DisplayName = "View Resolve Mode", meta = (Tooltip = "Defines how we convert all subject(s) in our subject to a final look at point when processing look at calculations."))
    TEnumAsByte<EViewTargetResolveModes> TargetResolveType;

    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Look", DisplayName = "Keep Subject(s) on Screen", meta = (Tooltip = "Will ensure that the resolved subjects do not leave the screen. NOTE: this can cause erratic and unpredictable camera motion as there is no damping applied to keeping targets on screen."))
    bool bKeepTargetOnScreen;

    UPROPERTY(EditAnywhere, Interp, Category = "Look", meta = (Tooltip = "Active only in Play mode. How long into the future to project the look at position based on the Subject's velocity. This can only be used when `First` subject combine mode, and `WorldSpace Center` resolve mode are selected.", ClampMin = 0.f, EditCondition = "TargetCombineMode == EViewTargeCombineModes::VTCM_First && TargetResolveType == EViewTargetResolveModes::VTRM_WorldCenter"))
    float VelocityLookAheadTime;

    UPROPERTY(EditAnywhere, Interp, Category = "Look", meta = (Tooltip = "Active only in Play mode. How much to smooth the velocity based look ahead. Larger values will suppress large changes in velocity, smoothing out the look ahead effect. This can only be used when `First` subject combine mode, and `WorldSpace Center` resolve mode are selected.", ClampMin = 0.f, EditCondition = "TargetCombineMode == EViewTargeCombineModes::VTCM_First && TargetResolveType == EViewTargetResolveModes::VTRM_WorldCenter"))
    float VelocityLookAheadDamping;

    UPROPERTY(EditAnywhere, Interp, Category = "Composition", DisplayName = "Subject Offset", meta = (Tooltip = "Adjusts the final look at point by this translation. Only possible when using `Worldspace Center` for subject resolve.", EditCondition = "TargetResolveType == EViewTargetResolveModes::VTRM_WorldCenter"))
    FVector TargetOffset;

    UPROPERTY(EditAnywhere, Interp, Category = "Composition", DisplayName = "Offset In Subject Local Space", meta = (Tooltip = "Selects whether or not the `Subject Offset` is relative to the local coordinate space of the subject. This can only be toggled when `First` subject combine mode, and `WorldSpace Center` resolve mode are selected.", EditCondition = "TargetCombineMode == EViewTargeCombineModes::VTCM_First && TargetResolveType == EViewTargetResolveModes::VTRM_WorldCenter"))
    bool bOffsetInTargetLocalSpace;

    UPROPERTY(EditAnywhere, Interp, Category = "Composition", DisplayName = "Subject Screen Position", meta = (ClampMin = 0.f, ClampMax = 1.f, Tooltip = "Specifies where on the screen you would like the center of the subjects to be. Bottom left is [0,0], and Top right is [1,1], and is independent of screen resolution or aspect ratio."))
    FVector2D ScreenPosition;

    UPROPERTY(EditAnywhere, Interp, Category = "Composition", meta = (ToolTip = "Damping settings for look at motion. Larger values make the component track slowly, and small values more quickly, i.e. 1 is quite slow, and 0.1 is quite fast."))
    FBlackEyeCompositionDamping Damping;

    UPROPERTY(EditAnywhere, Interp, Category = "Composition", DisplayName = "Subject Dead Zone", meta = (ClampMin = 0.f, CampMax = 1.f, Tooltip = "Create a screen space area which will ignore all subject motion inside it"))
    FVector2D ScreenDeadZoneSize;

    UPROPERTY(EditAnywhere, Interp, Category = "Dynamic FOV", DisplayName = "Enable Dynamic FoV", meta = (Tooltip = "Camera will automatically zoom between wide and telephoto limits to keep subject at desired Subject Viewport Size percentage"))
    bool bDynamicFoV;

    UPROPERTY(EditAnywhere, Interp, Category = "Dynamic FOV", DisplayName = "Telephoto Limit", meta = (EditConditionHides, EditCondition = "bDynamicFoV", ClampMin = 1.f, ClampMax = 180.f, Units = "deg"))
    float MinFoV;

    UPROPERTY(EditAnywhere, Interp, Category = "Dynamic FOV", DisplayName = "Wide Limit", meta = (EditConditionHides, EditCondition = "bDynamicFoV", ClampMin = 1.f, ClampMax = 179.f, Units = "deg"))
    float MaxFoV;

    UPROPERTY(EditAnywhere, Interp, Category = "Dynamic FOV", DisplayName = "FoV Damping", meta = (ClampMin = 0.f, EditCondition = "bDynamicFoV", EditConditionHides))
    float FieldOfViewDamping;

    UPROPERTY(EditAnywhere, Interp, Category = "Dynamic FOV", DisplayName = "Desired Subject Viewport Size", meta = (ClampMin = 0.f, ClampMax = 200.f, Units = "%", EditCondition = "bDynamicFoV", EditConditionHides))
    float DesiredTargetViewportSize;

    UPROPERTY(EditAnywhere, Interp, Category = "Focus", DisplayName = "Set Focal Distance", meta = (Tooltip = "Set the distance to the subject group as the focal distance. If a CineCamera exists, it must be set to manual focus mode."))
    bool bSetFocalDistance;

    UPROPERTY(EditAnywhere, Interp, Category = "Focus", DisplayName = "Focal Distance Offset", meta = (EditCondition = "bSetFocalDistance", Units = "cm", Tooltip = "Offset the position of the resolved subjects forward or backward this amount to fine tune depth of field/focus."))
    float FocalDistanceOffset;

    static FVector2D GetLookAtViewportPosition(EViewLookPosition Position);

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UFUNCTION(BlueprintCallable, Category = "Look At Subject(s)")
    int GetNumViewTargets() const;
    UFUNCTION(BlueprintCallable, Category = "Look At Subject(s)")
    void SetViewTargetCombineMode(EViewTargeCombineModes ResolveMode);
    // TODO: Cannot get the ref out variable to work in BP, so do not expose it
    bool GetTargetAtIndex(FBlackEyeTarget& Target, int Index) const;
    // Cannot reasonably call directly from BP due to how cumbersome setting up an FBlackEyeTarget can be
    UFUNCTION(BlueprintCallable, Category = "Look At Subject(s)")
    void RemoveTargetAtIndex(int Index);
    UFUNCTION(BlueprintCallable, Category = "Look At Subject(s)")
    virtual void ClearAllTargets() override;
    UFUNCTION(BlueprintCallable, Category = "Look At Subject(s)")
    virtual int GetNumTargets() override;

    // Cannot reasonably call directly from BP due to how cumbersome setting up an FBlackEyeTarget can be
    virtual int AddLookAt(FBlackEyeTarget Target, bool RequestSnap) override;;
    UFUNCTION(BlueprintCallable, Category = "Look At Target(s)", meta = (DisplayName = "Set Single Look At Target", useAutoSize = true))
    virtual int AddLookAt(class USceneComponent* Component, FString BoneName, bool UseAutoSize, float BoundingRadius, bool RequestSnap) override;
    UFUNCTION(BlueprintCallable, Category = "Look At Target(s)", meta = (DisplayName = "Remove Look At Target"))
    virtual bool RemoveTarget(int Index, bool RequestSnap) override;

    virtual void SnapToTargets() override;
    // Cannot reasonably call directly from BP due to how cumbersome setting up an FBlackEyeTarget can be
    virtual void SetLookAt(FBlackEyeTarget Target, int Index, bool RequestSnap) override;
    UFUNCTION(BlueprintCallable, Category = "Look At Target(s)", meta = (DisplayName = "Set Single Look At Target", useAutoSize = true))
    virtual void SetLookAt(class USceneComponent* Component, FString BoneName, int Index, bool UseAutoSize, float BoundingRadius, bool RequestSnap) override;
    UFUNCTION(BlueprintCallable, Category = "Look At Target(s)", meta = (DisplayName = "Set Single Look At Target Actor Override"))
    virtual void SetLookAtActorOverride(AActor* TargetActor, int Index, bool RequestSnap) override;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    bool GetLookAtMetrics(FVector FromPoint,
        const FRotator& WithRotation,
        FIntPoint ViewportSize,
        float ViewportAspectRatio,
        FVector& /* out */ OutResolvedLookVector,
        FBox& /* out */ OutWorldBox,
        FBox2D& /* out */ OutScreenSpaceBox) const;

    bool GetTargetGroupBoundingVolume(FBox& OutTargetBox) const;
    bool GetTargetGroupViewportBoundingBox(const FSceneViewProjectionData& ProjectionData, FBox2D& OutTargetBox, FBox& OutWorldBox) const;

private:
#if WITH_EDITOR
    friend class UBlackEyeCameraSubsystem;
    // NOTE: Look at takes into account screen target position
    FVector DesiredLookAtVectorWorld;
    FVector LastLookAtVectorWorld;
#endif

    // Last calculated rotation not taking into account screen target position (looking straight on)
    // TODO: Due to non-UPROPERTY() members being reset when an instance of a BP, this must be made a hidden UPROPERTY to ensure it persists through a change
    UPROPERTY(EditAnywhere, NonPIEDuplicateTransient, Category = "Hidden", meta = (HideInDetailPanel))
    FQuat LastRotationLocal;
    bool SnapRotation;

    bool bUseVelocityLookAhead;

    float PitchVelocity;
    float YawVelocity;
    float FieldOfViewVelocity;

    UPROPERTY(VisibleAnywhere, DisplayName = "Calculated FoV",  Category = "Dynamic FOV", meta = (EditConditionHides, EditCondition = "bDynamicFoV", DisplayAfter = "DesiredTargetViewportSize"))
    float RequestedFoV;

    FVector LookAheadVelocityVector;
    float LookAheadVelocityVelocity;

    void CacheRootCamActorComponent();
    bool CameraExistsInActor() const;

    const FViewport* GetLocalViewport() const;

    FBox2D GetTargetViewportBox() const;

    TWeakObjectPtr<ABlackEyeCameraRigBase> RootActorComponent;

    int SnapSettleFrameCounter;
};
