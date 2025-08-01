// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"

enum class ECameraFrustumDebugDrawMode : uint8 {
    CFDDM_None = 0,
    CFDDM_ActiveOnly = 1,
    CFDDM_AllFrustum = 2,
    CFDDM_AllButActiveFrustum = 3
};

class BLACK_EYE_API FBlackEyeCVars
{
public:
    static ECameraFrustumDebugDrawMode ShowCameraFrustums() { return (ECameraFrustumDebugDrawMode)CVarShowBlackEyeCameraFrustums.GetValueOnGameThread(); }
    static bool ShowCameraNames() { return CVarShowBlackEyeCameraNames.GetValueOnGameThread() == 1; }
    static bool DampingDisabled() { return CVarDisableDamping.GetValueOnGameThread() == 1; }

private:
    static TAutoConsoleVariable<int32> CVarShowBlackEyeCameraFrustums;
    static TAutoConsoleVariable<int32> CVarShowBlackEyeCameraNames;
    static TAutoConsoleVariable<int32> CVarDisableDamping;
};