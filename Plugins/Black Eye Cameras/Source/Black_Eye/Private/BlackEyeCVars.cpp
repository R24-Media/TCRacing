// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "BlackEyeCVars.h"

TAutoConsoleVariable<int32> FBlackEyeCVars::CVarShowBlackEyeCameraFrustums(
                                                TEXT("BlackEye.ShowCameraFrustums"),
                                                3,
                                                TEXT("Sets the debug frustum drawing state for Black Eye Camera actors (WARNING: Can slow performance in large scenes)\n")
                                                TEXT("0: off\n")
                                                TEXT("1: show debug camera frustums for active BET camera actor only\n")
                                                TEXT("2: show debug camera frustums for all BET camera actors\n")
                                                TEXT("3: show debug camera frustums for all BET camera actors except active\n"));

TAutoConsoleVariable<int32> FBlackEyeCVars::CVarShowBlackEyeCameraNames(
                                                TEXT("BlackEye.ShowCameraNames"),
                                                1,
                                                TEXT("Sets whether or not to draw the Camera Rig Actor's name when black eye cameras show flag is true\n")
                                                TEXT("0: off\n")
                                                TEXT("1: on"));

TAutoConsoleVariable<int32> FBlackEyeCVars::CVarDisableDamping(
                                                TEXT("BlackEye.DisableDamping"),
                                                0,
                                                TEXT("Disables smooth damping for all motion in Black Eye components (Follow & Look)\n")
                                                TEXT("0: Smooth damping enabled\n")
                                                TEXT("1: Smooth damping disabled"));
