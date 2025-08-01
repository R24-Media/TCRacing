// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "Runtime/Engine/Classes/Debug/DebugDrawService.h"

#include "BlackEyeCameraSubsystem.generated.h"

/**
 * Subsystem used by the BlackEye plugin to draw useful debug information about its components & actors into the game world
 */
UCLASS()
class BLACK_EYE_API UBlackEyeCameraSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickableInEditor() const override { return true; }

private:
#if WITH_EDITOR && UE_ENABLE_DEBUG_DRAWING
    void DrawDebugUI(class UCanvas* Canvas, class APlayerController* Controller);

    static void DrawFollowComponentDebugHUD(class FCanvas* Canvas, class UWorld* World, const class UCameraComponent* Camera, class UFollowComponent* CamRigComp);
    static void DrawLookAtComponentDebugHUD(class UCanvas* Canvas, class UWorld* World, const class UCameraComponent* Camera, class ULookAtComponent* LookAtComp);
    static void DrawFollowComponentVisualization(class UFollowComponent* Follow, class UWorld* World, class UCameraComponent* Camera);
    static void DrawLookAtComponentVisualization(class ULookAtComponent* LookAt, class UWorld* World, class UCameraComponent* Camera);

    class IConsoleVariable* ShowBlackEyeCameraFlag;
    FDelegateHandle DebugDrawDelegateHandle;
    FDebugDrawDelegate DebugDrawDelegate;
#endif
};
