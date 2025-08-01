// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"

class BLACK_EYE_API FBlackEyeMath
{
public:
#if ENGINE_MINOR_VERSION <= 3
    UE_NODISCARD 
#else
    [[nodiscard]]
#endif
        /**
        * Returns a new vector, smoothly moving towards the Target vector with each of the 3 components of the vector all traveling at different velocities.
        * @param Current - Current position
        * @param Target -  Target position
        * @param OutPiecewiseVelocities - The velocities for each of the 3 components of the vector
        * @param SmoothTimes - The smooth damping time values for each component of the vector
        * @param DeltaTime - The delta time for this iteration
        * @param MaxSpeed - The maximum speed for any of the components of the vector to move towards the target
        */
        static FVector SmoothDamp(FVector Current, FVector Target, FVector& OutPiecewiseVelocities, FVector SmoothTimes, float DeltaTime, float MaxSpeed = INFINITY);

#if ENGINE_MINOR_VERSION <= 3
        UE_NODISCARD
#else
        [[nodiscard]]
#endif
        /**
        * Returns a new vector, smoothly moving towards the Target vector as a single point.
        * @param Current - Current position
        * @param Target -  Target position
        * @param OutVelocity - The velocity for the motion of the current vector to its target
        * @param SmoothTime - The smooth damping time value for the vector
        * @param DeltaTime - The delta time for this iteration
        * @param MaxSpeed - The maximum speed for the vector to move towards the target
        */
        static FVector SmoothDamp(FVector Current, FVector Target, float& /* out */ OutVelocity, float SmoothTime, float DeltaTime, float MaxSpeed = INFINITY);

#if ENGINE_MINOR_VERSION <= 3
        UE_NODISCARD
#else
        [[nodiscard]]
#endif
        /**
        * Returns a new quaternion, smoothly moving towards the Target quaternion using spherical motion.
        * @param Current - Current position
        * @param Target -  Target position
        * @param OutAngularVelocity - The angular velocity for the motion of the current quaternion to its target
        * @param SmoothTime - The smooth damping time value for the quaternion
        * @param DeltaTime - The delta time for this iteration
        * @param MaxSpeed - The maximum speed for the quaternion's angle to move towards the target
        */
        static FQuat SmoothDamp(FQuat Current, FQuat Target, float& /* out */ OutAngularVelocity, float SmoothTime, float DeltaTime, float MaxSpeed = INFINITY);

#if ENGINE_MINOR_VERSION <= 3
        UE_NODISCARD
#else
        [[nodiscard]]
#endif
        /**
        * Returns a new value, smoothly moving towards the Target value using damped motion.
        * @param Current - Current value
        * @param Target -  Target value
        * @param OutVelocity - The velocity for the motion of the current value to its target
        * @param SmoothTime - The smooth damping time value for the value
        * @param DeltaTime - The delta time for this iteration
        * @param MaxSpeed - The maximum speed for the value to move towards the target
        */
        static float SmoothDamp(float Current, float Target, float& /* out */ OutVelocity, float SmoothTime, float DeltaTime, float MaxSpeed = INFINITY);

#if ENGINE_MINOR_VERSION <= 3
        UE_NODISCARD
#else
        [[nodiscard]]
#endif
        /**
        * Returns a new angle, smoothly moving towards the target angle using damped motion along the shortest path on a circle.
        * @param CurrentAngleDeg - Current angle
        * @param TargetAngleDeg -  Target ngle
        * @param OutVelocity - The velocity for the motion of the current angle to its target
        * @param SmoothTime - The smooth damping time value for the value
        * @param DeltaTime - The delta time for this iteration
        * @param MaxSpeed - The maximum speed for the angle to move towards the target
        */
        static float SmoothDampAngle(float CurrentAngleDeg, float TargetAngleDeg, float& /* out */ OutVelocity, float SmoothTime, float DeltaTime, float MaxSpeed = INFINITY);

#if ENGINE_MINOR_VERSION <= 3
        UE_NODISCARD
#else
        [[nodiscard]]
#endif
        /**
        * Returns a constrained inverse lerp in the range of [0,1] for a value between the min and max value specified
        * @param Value - The value to convert into an inverse lerp
        * @param Min -  The lower limit for the range
        * @param Max - The upper limit for the range
        */
        static FORCEINLINE float InverseLerp(float Value, float Min, float Max)
    {
        return FMath::Clamp((Value - Min) / (Max - Min), 0.f, 1.f);
    }


#if ENGINE_MINOR_VERSION <= 3
        UE_NODISCARD
#else
        [[nodiscard]]
#endif
    /**
    * Builds a look-at quaternion with roll based on an up vector
    * @param LookAt - The look direction for the quaternion
    * @param UpDirection -  The up vector for the quaternion which is used to calculate roll
    */
        static FORCEINLINE FQuat MakeLookAt(const FVector& LookAt, const FVector& UpDirection)
    {
        return FRotationMatrix::MakeFromXZ(LookAt, UpDirection).ToQuat();
    }

#if ENGINE_MINOR_VERSION <= 3
        UE_NODISCARD
#else
        [[nodiscard]]
#endif
        static FORCEINLINE float AngleRepeat(float value)
    {
        value += 180.f;
        value = FMath::Fmod(value, 360.f);
        return value - 180.f;
    }

    /**
    * Projects the WorldPosition into viewport space ([0,1] range for XY axes)
    * @param WorldPosition - The world position to project
    * @param ViewProjectionMatrix - The view * projection matrix to use for this projection
    * @param OutScreenPos - The position in 2D viewport space of the projected point
    */
    static void ProjectWorldToViewport(const FVector& WorldPosition, const FMatrix& ViewProjectionMatrix, FVector2D& OutScreenPos);

    /**
    * Deprojects the viewport position ([0,1] range for XY axes) into world space, as well its view ray. Modified from UE source which implicitly converted to screenspace pixel coordinates
    * @param ViewportPos - The viewport position to deproject
    * @param InvViewProjMatrix - The inverse view * projection matrix to use for this deprojection
    * @param OutScreenPos - The position in 2D viewport space of the projected point
    * @param OutWorldOrigin - The origin of the point 
    * @param OutScreenPos - The view ray from the world origin at the specific viewport position
    */
    static void DeprojectViewportToWorld(const FVector2D& ViewportPos, const FMatrix& InvViewProjMatrix, FVector& OutWorldOrigin, FVector& out_WorldDirection);
};
