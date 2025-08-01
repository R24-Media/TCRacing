// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Camera/CameraComponent.h"

#include "BlackEyeCameraRigBase.generated.h"

UCLASS(ClassGroup = "BlackEye", DisplayName = "Black Eye Base Camera Rig", hideCategories = (Cooking, Input, Rendering, Navigation, LOD))
class BLACK_EYE_API ABlackEyeCameraRigBase : public AActor
{
    GENERATED_BODY()
    
public:	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Auto Assign", meta = (Tooltip = "The Local player index we wish to assign this camera to."))
    TEnumAsByte<EAutoReceiveInput::Type> AutoAssignTo;

    UPROPERTY(EditAnywhere, Category = "Camera Auto Assign", DisplayName = "Assign to Look At", meta = (EditCondition = "AutoAssignTo != EAutoReceiveInput::Disabled", EditConditionHides))
    bool bSetPawnAsLookAt;

    UPROPERTY(EditAnywhere, Category = "Camera Auto Assign", DisplayName = "Assign to Follow", meta = (EditCondition = "AutoAssignTo != EAutoReceiveInput::Disabled", EditConditionHides))
    bool bSetPawnAsFollow;

    ABlackEyeCameraRigBase();

    /**
    * Signals to the Black Eye components within this actor that they should snap directly to their tagets based on their configuration next frame
    */
    UFUNCTION(BlueprintCallable, Category = "Black Eye Camera Rig")
    void SnapComponentsToTargets();

    virtual bool ShouldTickIfViewportsOnly() const override;
    virtual void Tick(float DeltaTime) override;
    TWeakObjectPtr<UCameraComponent> GetCamera() const;

#if WITH_EDITOR
    virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown);
#endif

protected:
    virtual void BeginPlay() override;

private:
    TWeakObjectPtr<UCameraComponent> Camera;

    bool TryGetCameraComponent();

};
