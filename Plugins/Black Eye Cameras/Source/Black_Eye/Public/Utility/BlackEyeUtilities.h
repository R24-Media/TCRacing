// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "SceneView.h"

#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

class BLACK_EYE_API FBlackEyeUtilities
{
public:
    static FSceneViewProjectionData GetViewProjectionData(class UCameraComponent* Camera, 
                                                            FVector ViewOrigin, 
                                                            FRotator ViewRotation, 
                                                            FIntPoint ViewportSize, 
                                                            float AspectRatio = 1.7778f, 
                                                            float DesiredFoV = -1.f);

    static FVector2D GetScreenPoint(const FSceneViewProjectionData& ProjectionData, FVector WorldPoint);

    static FVector2D GetScreenPoint(const FIntRect& ViewRect, const FMatrix& ViewProjection, FVector WorldPoint);

    static FVector2D GetViewportPoint(const FSceneViewProjectionData& ProjectionData, FVector WorldPoint);

    static FVector2D GetViewportPoint(const FMatrix& ViewProjection, FVector WorldPoint);

    static FVector GetWorldVectorFromViewportPoint(const FSceneViewProjectionData& ProjectionData, FVector2D ViewportPosition);

#if ENGINE_MINOR_VERSION < 2
    static FVector SlerpNormals(const FVector& NormalA, const FVector& NormalB, double Alpha);
#endif

    static void DrawDebugQuad(const class UWorld* World,
                                const FVector& BotLeft,
                                const FVector& TopLeft,
                                const FVector& TopRight,
                                const FVector& BotRight,
                                const FColor& Color = FColor::Green,
                                const bool bPersistentLines = false,
                                const float LifeTime = -1.0f,
                                const uint8_t DepthPriority = 0,
                                const float Thickness = 0.0f);

    static void DrawDebugFrustum(const class UWorld* World, 
                                    const FMatrix& ViewProjectionMatrix, 
                                    const float ZNear, const float ZFar, 
                                    float AspectRatio = 1.0f, 
                                    const FColor& Color = FColor::Green, 
                                    const bool bPersistentLines = false, 
                                    const float LifeTime = -1.0f, 
                                    const uint8_t DepthPriority = 0, 
                                    float Thickness = 0.0f);
};
