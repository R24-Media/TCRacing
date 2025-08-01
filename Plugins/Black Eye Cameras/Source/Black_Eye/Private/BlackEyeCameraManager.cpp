// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "BlackEyeCameraManager.h"

#include "Runtime/Engine/Classes/GameFramework/Pawn.h"
#include "Engine/World.h"

void ABlackEyeCameraManager::DoUpdateCamera(float DeltaTime)
{
    // TODO: Do not call super() on this. Provide our own custom camera logic
    Super::DoUpdateCamera(DeltaTime);


    return;
    //FMinimalViewInfo NewPOV = ViewTarget.POV;

    //// update color scale interpolation
    //if (bEnableColorScaleInterp)
    //{
    //    float BlendPct = FMath::Clamp((GetWorld()->TimeSeconds - ColorScaleInterpStartTime) / ColorScaleInterpDuration, 0.f, 1.0f);
    //    ColorScale = FMath::Lerp(OriginalColorScale, DesiredColorScale, BlendPct);
    //    // if we've maxed
    //    if (BlendPct == 1.0f)
    //    {
    //        // disable further interpolation
    //        bEnableColorScaleInterp = false;
    //    }
    //}

    //// Don't update outgoing viewtarget during an interpolation when bLockOutgoing is set.
    //if ((PendingViewTarget.Target == NULL) || !BlendParams.bLockOutgoing)
    //{
    //    // Update current view target
    //    ViewTarget.CheckViewTarget(PCOwner);
    //    UpdateViewTarget(ViewTarget, DeltaTime);
    //}

    //// our camera is now viewing there
    //NewPOV = ViewTarget.POV;
    //// if we have a pending view target, perform transition from one to another.
    //if (PendingViewTarget.Target != NULL)
    //{
    //    BlendTimeToGo -= DeltaTime;

    //    // Update pending view target
    //    PendingViewTarget.CheckViewTarget(PCOwner);
    //    UpdateViewTarget(PendingViewTarget, DeltaTime);

    //    // blend....
    //    if (BlendTimeToGo > 0)
    //    {
    //        float DurationPct = (BlendParams.BlendTime - BlendTimeToGo) / BlendParams.BlendTime;

    //        float BlendPct = 0.f;
    //        switch (BlendParams.BlendFunction)
    //        {
    //        case VTBlend_Linear:
    //            BlendPct = FMath::Lerp(0.f, 1.f, DurationPct);
    //            break;
    //        case VTBlend_Cubic:
    //            BlendPct = FMath::CubicInterp(0.f, 0.f, 1.f, 0.f, DurationPct);
    //            break;
    //        case VTBlend_EaseIn:
    //            BlendPct = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPct, BlendParams.BlendExp));
    //            break;
    //        case VTBlend_EaseOut:
    //            BlendPct = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPct, 1.f / BlendParams.BlendExp));
    //            break;
    //        case VTBlend_EaseInOut:
    //            BlendPct = FMath::InterpEaseInOut(0.f, 1.f, DurationPct, BlendParams.BlendExp);
    //            break;
    //        case VTBlend_PreBlended:
    //            BlendPct = 1.0f;
    //            break;
    //        default:
    //            break;
    //        }

    //        UE_LOG(LogTemp, Display, TEXT("Blendign! %.3f"), BlendPct);

    //        //if (ACameraRig* currTarget = Cast<ACameraRig>(ViewTarget.Target.Get()))
    //        {
    //            //NewPOV = ViewTarget.POV;
    //        }
    //      //  else
    //        {
    //            // Update pending view target blend
    //            NewPOV = ViewTarget.POV;
    //            NewPOV.BlendViewInfo(PendingViewTarget.POV, BlendPct);//@TODO: CAMERA: Make sure the sense is correct!  BlendViewTargets(ViewTarget, PendingViewTarget, BlendPct);
    //        }

    //        // Add this pending view target's post-process settings as an override of the main view target's one,
    //        // since it is blending on top of it.
    //        const float PendingViewTargetPPWeight = PendingViewTarget.POV.PostProcessBlendWeight * BlendPct;
    //        if (PendingViewTargetPPWeight > 0.f)
    //        {
    //            AddCachedPPBlend(PendingViewTarget.POV.PostProcessSettings, PendingViewTargetPPWeight, VTBlendOrder_Override);
    //        }
    //    }
    //    else
    //    {
    //        // we're done blending, set new view target
    //        ViewTarget = PendingViewTarget;

    //        // clear pending view target
    //        PendingViewTarget.Target = NULL;

    //        BlendTimeToGo = 0;

    //        // our camera is now viewing there
    //        NewPOV = PendingViewTarget.POV;

    //        OnBlendComplete().Broadcast();
    //    }
    //}

    //if (bEnableFading)
    //{
    //    if (bAutoAnimateFade)
    //    {
    //        FadeTimeRemaining = FMath::Max(FadeTimeRemaining - DeltaTime, 0.0f);
    //        if (FadeTime > 0.0f)
    //        {
    //            FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining / FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
    //        }

    //        if ((bHoldFadeWhenFinished == false) && (FadeTimeRemaining <= 0.f))
    //        {
    //            // done
    //            StopCameraFade();
    //        }
    //    }

    //    if (bFadeAudio)
    //    {
    //        ApplyAudioFade();
    //    }
    //}

    //if (AllowPhotographyMode())
    //{
    //    const bool bPhotographyCausedCameraCut = UpdatePhotographyCamera(NewPOV);
    //    bGameCameraCutThisFrame = bGameCameraCutThisFrame || bPhotographyCausedCameraCut;
    //}

    //// Cache results
    //FillCameraCache(NewPOV);
}
