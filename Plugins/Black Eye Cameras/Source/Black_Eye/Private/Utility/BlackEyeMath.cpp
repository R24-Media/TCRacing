// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "Utility/BlackEyeMath.h"

FVector FBlackEyeMath::SmoothDamp(FVector Current,
                                    FVector Target, 
                                    FVector& OutPiecewiseVelocities, 
                                    FVector SmoothTimes, 
                                    float DeltaTime, 
                                    float MaxSpeed)
{
    FVector SmoothDampedResult;

    float TempVelocity = OutPiecewiseVelocities.X;
    SmoothDampedResult.X = SmoothDamp(Current.X, Target.X, TempVelocity, SmoothTimes.X, DeltaTime, MaxSpeed);
    OutPiecewiseVelocities.X = TempVelocity;

    TempVelocity = OutPiecewiseVelocities.Y;
    SmoothDampedResult.Y = SmoothDamp(Current.Y, Target.Y, TempVelocity, SmoothTimes.Y, DeltaTime, MaxSpeed);
    OutPiecewiseVelocities.Y = TempVelocity;

    TempVelocity = OutPiecewiseVelocities.Z;
    SmoothDampedResult.Z = SmoothDamp(Current.Z, Target.Z, TempVelocity, SmoothTimes.Z, DeltaTime, MaxSpeed);
    OutPiecewiseVelocities.Z = TempVelocity;

    return SmoothDampedResult;
}

FVector FBlackEyeMath::SmoothDamp(FVector Current, 
                                    FVector Target, 
                                    float& /* out */ OutVelocity, 
                                    float SmoothTime, 
                                    float DeltaTime, 
                                    float MaxSpeed)
{
    FVector Delta = Target - Current;
    float Distance = Delta.Length();

    //Choose a reasonable epsilon here: SMALL_NUMBER is too small
    if (Distance > UE_KINDA_SMALL_NUMBER)
    {
        Distance = SmoothDamp(0.f, Distance, OutVelocity, SmoothTime, DeltaTime, MaxSpeed);
    }
    else
    {
        OutVelocity = 0.f;
        return Target;
    }

    Delta.Normalize();
    return Current + (Delta * Distance);
}

FQuat FBlackEyeMath::SmoothDamp(FQuat Current,
                    FQuat Target, 
                    float& /* out */ OutAngularVelocity, 
                    float SmoothTime, 
                    float DeltaTime, 
                    float MaxSpeed)
{
    float QuatAngularDistance = Current.AngularDistance(Target);
    float NewQuatAngularDistance = FBlackEyeMath::SmoothDamp(QuatAngularDistance, 0.f, OutAngularVelocity, SmoothTime, DeltaTime);

    //Choose a reasonable epsilon here: SMALL_NUMBER is too small
    if (QuatAngularDistance > 0.0001f)
    {
        float tValue = 1.f - NewQuatAngularDistance / QuatAngularDistance;
        return FQuat::Slerp(Current, Target, tValue);
    }

    OutAngularVelocity = 0.f;
    return Target;
}

float FBlackEyeMath::SmoothDamp(float Current, 
                                float Target, 
                                float& /* out */ OutVelocity, 
                                float SmoothTime, 
                                float DeltaTime, 
                                float MaxSpeed)
{
    float targetVel = 0.f;
    FMath::CriticallyDampedSmoothing(Current, OutVelocity, Target, targetVel, DeltaTime, SmoothTime);
    return Current;
}

float FBlackEyeMath::SmoothDampAngle(float CurrentAngleDeg, 
                                        float TargetAngleDeg, 
                                        float& /* out */ OutVelocity, 
                                        float SmoothTime, 
                                        float DeltaTime, 
                                        float MaxSpeed)
{
    TargetAngleDeg = CurrentAngleDeg + FMath::FindDeltaAngleDegrees(CurrentAngleDeg, TargetAngleDeg);
    return SmoothDamp(CurrentAngleDeg, TargetAngleDeg, OutVelocity, SmoothTime, DeltaTime, MaxSpeed);
}

void FBlackEyeMath::ProjectWorldToViewport(const FVector& WorldPosition, const FMatrix& ViewProjectionMatrix, FVector2D& OutScreenPos)
{
    FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
    if (Result.W > 0.0f)
    {
        // the result of this will be x and y coords in -1..1 projection space
        const float RHW = 1.0f / Result.W;
        FPlane PosInScreenSpace = FPlane(Result.X * RHW, Result.Y * RHW, Result.Z * RHW, Result.W);

        // Move from projection space to normalized 0..1 UI space. (0,0) being bottom left
        const float NormalizedX = (PosInScreenSpace.X / 2.f) + 0.5f;
        const float NormalizedY = (PosInScreenSpace.Y / 2.f) + 0.5f;

        OutScreenPos = FVector2D(NormalizedX, NormalizedY);
    }
}

void FBlackEyeMath::DeprojectViewportToWorld(const FVector2D& ViewportPos, const FMatrix& InvViewProjMatrix, FVector& OutWorldOrigin, FVector& out_WorldDirection)
{
    // Get the pixel coordinates into -1..1 projection space
    const float ScreenSpaceX = (ViewportPos.X - 0.5f) * 2.0f;
    const float ScreenSpaceY = (ViewportPos.Y - 0.5f) * 2.0f;

    // The start of the ray trace is defined to be at mousex,mousey,1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
    // To get the direction of the ray trace we need to use any z between the near and the far plane, so let's use (mousex, mousey, 0.01)
    const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
    const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.01f, 1.0f);

    // Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
    // by the projection matrix should use homogeneous coordinates (i.e. FPlane).
    const FVector4 HGRayStartWorldSpace = InvViewProjMatrix.TransformFVector4(RayStartProjectionSpace);
    const FVector4 HGRayEndWorldSpace = InvViewProjMatrix.TransformFVector4(RayEndProjectionSpace);
    FVector RayStartWorldSpace(HGRayStartWorldSpace.X, HGRayStartWorldSpace.Y, HGRayStartWorldSpace.Z);
    FVector RayEndWorldSpace(HGRayEndWorldSpace.X, HGRayEndWorldSpace.Y, HGRayEndWorldSpace.Z);
    // divide vectors by W to undo any projection and get the 3-space coordinate
    if (HGRayStartWorldSpace.W != 0.0f)
    {
        RayStartWorldSpace /= HGRayStartWorldSpace.W;
    }
    if (HGRayEndWorldSpace.W != 0.0f)
    {
        RayEndWorldSpace /= HGRayEndWorldSpace.W;
    }
    const FVector RayDirWorldSpace = (RayEndWorldSpace - RayStartWorldSpace).GetSafeNormal();

    // Finally, store the results in the outputs
    OutWorldOrigin = RayStartWorldSpace;
    out_WorldDirection = RayDirWorldSpace;
}