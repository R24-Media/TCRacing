// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "Utility/BlackEyeUtilities.h"

#include "Utility/BlackEyeMath.h"

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"

#include "DrawDebugHelpers.h"

FSceneViewProjectionData FBlackEyeUtilities::GetViewProjectionData(UCameraComponent* Camera, 
                                                                    FVector ViewOrigin, 
                                                                    FRotator ViewRotation, 
                                                                    FIntPoint ViewportSize, 
                                                                    float AspectRatio, 
                                                                    float DesiredFoV)
{
    // Offset look rotation by view offset, assume not NULL because of checks in Tick()
    FMinimalViewInfo ViewInf = FMinimalViewInfo();
    Camera->GetCameraView(0.f, ViewInf);

    if (DesiredFoV > 0.f)
    {
        ViewInf.DesiredFOV = DesiredFoV;
    }

    ViewInf.AspectRatio = AspectRatio;

    FSceneViewProjectionData ProjectionData = FSceneViewProjectionData();

    ProjectionData.ViewOrigin = ViewOrigin;
    ProjectionData.ViewRotationMatrix = FInverseRotationMatrix(ViewRotation) * FMatrix(FPlane(0, 0, 1, 0),
        FPlane(1, 0, 0, 0),
        FPlane(0, 1, 0, 0),
        FPlane(0, 0, 0, 1));
    ProjectionData.ProjectionMatrix = ViewInf.CalculateProjectionMatrix();
    FIntRect ViewRect = FIntRect(0, 0, ViewportSize.X, ViewportSize.Y);

    ProjectionData.SetViewRectangle(ViewRect);

    if (Camera->bConstrainAspectRatio)
    {
        // calculate constrained view rect
        float ViewportAspectRatio = (float)ViewportSize.X / (float)ViewportSize.Y;
        FIntRect ConstrainedViewRect = ViewRect;

        if (ViewportAspectRatio < Camera->AspectRatio)
        {
            // Need to make constraint shorter
            float newHeight = ViewRect.Width() / Camera->AspectRatio;
            float halfDelta = (ViewRect.Height() - newHeight) * 0.5f;
            ConstrainedViewRect.Min.Y += halfDelta;
            ConstrainedViewRect.Max.Y -= halfDelta;
        }
        else
        {
            // Need to make constraint thinner
            float newWidth = ViewRect.Height() * Camera->AspectRatio;
            float halfDelta = (ViewRect.Width() - newWidth) * 0.5f;
            ConstrainedViewRect.Min.X += halfDelta;
            ConstrainedViewRect.Max.X -= halfDelta;
        }

        //UE_LOG(LogTemp, Display, TEXT("Reduced aspect to: %.3f (requested %.3f"), (float)constrainedViewRect.Width() / (float)constrainedViewRect.Height(), Camera->AspectRatio)
        ProjectionData.SetConstrainedViewRectangle(ConstrainedViewRect);
    }

    return ProjectionData;
}

FVector2D FBlackEyeUtilities::GetScreenPoint(const FSceneViewProjectionData& ProjectionData, FVector WorldPoint)
{
    return GetScreenPoint(ProjectionData.GetViewRect(), ProjectionData.ComputeViewProjectionMatrix(), WorldPoint);
}

FVector2D FBlackEyeUtilities::GetScreenPoint(const FIntRect& ViewRect, const FMatrix& ViewProjection, FVector WorldPoint)
{
    FVector2D viewRectSize = ViewRect.Size();
    FVector2D viewportPoint = GetViewportPoint(ViewProjection, WorldPoint);
    return FVector2D(ViewRect.Min.X + viewportPoint.X * viewRectSize.X, ViewRect.Min.Y + viewportPoint.Y * viewRectSize.Y);
}

FVector2D FBlackEyeUtilities::GetViewportPoint(const FSceneViewProjectionData& ProjectionData, FVector WorldPoint)
{
    return GetViewportPoint(ProjectionData.ComputeViewProjectionMatrix(), WorldPoint);
}

FVector2D FBlackEyeUtilities::GetViewportPoint(const FMatrix& ViewProjection, FVector WorldPoint)
{
    FVector2D viewportPoint;
    FBlackEyeMath::ProjectWorldToViewport(WorldPoint, ViewProjection, viewportPoint);

    return viewportPoint;
}

FVector FBlackEyeUtilities::GetWorldVectorFromViewportPoint(const FSceneViewProjectionData& ProjectionData, FVector2D ViewportPosition)
{
    FMatrix invViewProjMatrix = ProjectionData.ComputeViewProjectionMatrix().InverseFast();

    FVector worldPosition;
    FVector worldDirection;
    FBlackEyeMath::DeprojectViewportToWorld(ViewportPosition, invViewProjMatrix, /*out*/ worldPosition, /*out*/ worldDirection);

    return worldDirection;
}

#if ENGINE_MINOR_VERSION < 2
FVector FBlackEyeUtilities::SlerpNormals(const FVector& NormalA, const FVector& NormalB, double Alpha)
{
    // Find rotation from A to B
    const FQuat RotationQuat = FQuat::FindBetweenNormals(NormalA, NormalB);
    const FVector Axis = RotationQuat.GetRotationAxis();
    const double AngleRads = RotationQuat.GetAngle();

    // Rotate from A toward B using portion of the angle specified by Alpha.
    const FQuat DeltaQuat(Axis, AngleRads * Alpha);
    FVector Result = DeltaQuat.RotateVector(NormalA);
    return Result;
}
#endif

void FBlackEyeUtilities::DrawDebugQuad(const class UWorld* World, 
                                        const FVector& BotLeft, 
                                        const FVector& TopLeft, 
                                        const FVector& TopRight, 
                                        const FVector& BotRight, 
                                        const FColor& Color, 
                                        const bool bPersistentLines, 
                                        const float LifeTime, 
                                        const uint8_t DepthPriority, 
                                        const float Thickness)
{
    ::DrawDebugLine(World, BotLeft, TopLeft, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    ::DrawDebugLine(World, TopLeft, TopRight, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    ::DrawDebugLine(World, TopRight, BotRight, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    ::DrawDebugLine(World, BotRight, BotLeft, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
}

void FBlackEyeUtilities::DrawDebugFrustum(const class UWorld* World, 
                                            const FMatrix& ViewProjectionMatrix, 
                                            const float ZNear, 
                                            const float ZFar, 
                                            float AspectRatio, 
                                            const FColor& Color, 
                                            const bool bPersistentLines, 
                                            const float LifeTime, 
                                            const uint8_t DepthPriority, 
                                            const float Thickness)
{
    const FMatrix InverseViewProjectionMatrix = ViewProjectionMatrix.Inverse();

    const float top = 1.0f * AspectRatio;
    const float bot = -1.0f * AspectRatio;
    const float left = -1.0f;
    const float right = 1.0f;
    const float ZNearMod = 1.0f / ZNear * 10.0f;
    const float ZFarMod = 1.0f / ZFar * 10.0f;

    FVector4 NearTopLeft = InverseViewProjectionMatrix.TransformPosition(FVector4(left, top, ZNearMod, 1));
    NearTopLeft /= FVector4(NearTopLeft.W, NearTopLeft.W, NearTopLeft.W, NearTopLeft.W);

    FVector4 NearTopRight = InverseViewProjectionMatrix.TransformPosition(FVector4(right, top, ZNearMod, 1));
    NearTopRight /= FVector4(NearTopRight.W, NearTopRight.W, NearTopRight.W, NearTopRight.W);

    FVector4 NearBotLeft = InverseViewProjectionMatrix.TransformPosition(FVector4(left, bot, ZNearMod, 1));
    NearBotLeft /= FVector4(NearBotLeft.W, NearBotLeft.W, NearBotLeft.W, NearBotLeft.W);

    FVector4 NearBotRight = InverseViewProjectionMatrix.TransformPosition(FVector4(right, bot, ZNearMod, 1));
    NearBotRight /= FVector4(NearBotRight.W, NearBotRight.W, NearBotRight.W, NearBotRight.W);

    FVector4 FarTopLeft = InverseViewProjectionMatrix.TransformPosition(FVector4(left, top, ZFarMod, 1));
    FarTopLeft /= FVector4(FarTopLeft.W, FarTopLeft.W, FarTopLeft.W, FarTopLeft.W);

    FVector4 FarTopRight = InverseViewProjectionMatrix.TransformPosition(FVector4(right, top, ZFarMod, 1));
    FarTopRight /= FVector4(FarTopRight.W, FarTopRight.W, FarTopRight.W, FarTopRight.W);

    FVector4 FarBotLeft = InverseViewProjectionMatrix.TransformPosition(FVector4(left, bot, ZFarMod, 1));
    FarBotLeft /= FVector4(FarBotLeft.W, FarBotLeft.W, FarBotLeft.W, FarBotLeft.W);

    FVector4 FarBotRight = InverseViewProjectionMatrix.TransformPosition(FVector4(right, bot, ZFarMod, 1));
    FarBotRight /= FVector4(FarBotRight.W, FarBotRight.W, FarBotRight.W, FarBotRight.W);

    // Near-far plane quads lines
    DrawDebugQuad(World, NearBotLeft, NearTopLeft, NearTopRight, NearBotRight, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawDebugQuad(World, FarBotLeft, FarTopLeft, FarTopRight, FarBotRight, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);

    // Near-to-far lines
    ::DrawDebugLine(World, NearTopLeft, FarTopLeft, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    ::DrawDebugLine(World, NearTopRight, FarTopRight, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    ::DrawDebugLine(World, NearBotLeft, FarBotLeft, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    ::DrawDebugLine(World, NearBotRight, FarBotRight, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
}